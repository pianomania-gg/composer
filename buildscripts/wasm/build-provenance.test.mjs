// SPDX-License-Identifier: GPL-3.0-only
// Exercise build.sh with a fake compiler: source reporting must identify the
// input revision, warn about all local source changes, and hash the output.
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { mkdtempSync, mkdirSync, readFileSync, rmSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { dirname, join, resolve } from 'node:path';
import { spawnSync } from 'node:child_process';

const buildScript = readFileSync(new URL('./build.sh', import.meta.url), 'utf8');
const bash = process.env.WASM_TEST_BASH || 'bash';
const wasm = 'fixture converter binary\n';
const hash = createHash('sha256').update(wasm).digest('hex');
const portable = path => path.replaceAll('\\', '/').replace(/^([A-Za-z]):/, (_, drive) => `/${drive.toLowerCase()}`);

for (const state of ['clean', 'unstaged', 'staged', 'untracked', 'missing-commit']) {
    const temp = mkdtempSync(join(tmpdir(), 'pm-wasm-provenance-'));
    const repo = join(temp, 'source tree');
    const put = (path, contents) => {
        mkdirSync(dirname(path), { recursive: true });
        writeFileSync(path, contents, { mode: 0o755 });
    };
    const git = (...args) => {
        const result = spawnSync('git', ['-C', repo, ...args], { encoding: 'utf8' });
        assert.equal(result.status, 0, result.stderr);
        return result.stdout.trim();
    };
    try {
        put(join(repo, 'buildscripts/wasm/build.sh'), buildScript);
        put(join(repo, 'source.cpp'), 'original\n');
        put(join(repo, '.gitignore'), '/build.wasm/\n');
        git('init');
        let commit;
        if (state !== 'missing-commit') {
            git('add', '.');
            git('-c', 'user.name=Provenance test', '-c', 'user.email=test@example.invalid',
                '-c', 'commit.gpgsign=false', 'commit', '-m', 'Fixture source');
            commit = git('rev-parse', 'HEAD');
        }
        if (state === 'unstaged' || state === 'staged') {
            put(join(repo, 'source.cpp'), 'changed\n');
            if (state === 'staged') git('add', 'source.cpp');
        } else if (state === 'untracked') {
            put(join(repo, 'new-source.cpp'), 'new\n');
        }

        put(join(temp, 'bin/emcmake'), '#!/usr/bin/env bash\nexit 0\n');
        put(join(temp, 'bin/cmake'), `#!/usr/bin/env bash
set -eu
if [ "$1" = "--build" ]; then
    mkdir -p "$2/public_html"
    printf 'fixture loader\\n' > "$2/public_html/pm-converter.js"
    printf 'fixture converter binary\\n' > "$2/public_html/pm-converter.wasm"
    # Generated files must not change the recorded pre-build source state.
    touch "$2/../generated-during-build.txt"
fi
`);
        put(join(temp, 'emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake'), '');
        put(join(temp, 'qt/lib/cmake/Qt6/qt.toolchain.cmake'), '');
        put(join(temp, 'qt/bin/qmake6'), '');
        mkdirSync(join(temp, 'qt-host'));

        const result = spawnSync(bash, ['-lc',
            'export PATH="$TEST_BIN:$PATH"; bash "source tree/buildscripts/wasm/build.sh"'], {
            cwd: temp,
            encoding: 'utf8',
            env: {
                ...process.env,
                TEST_BIN: portable(join(temp, 'bin')),
                EMSDK: portable(join(temp, 'emsdk')),
                QT_WASM_DIR: portable(join(temp, 'qt')),
                QT_HOST_DIR: portable(join(temp, 'qt-host')),
                PM_WASM_OUTPUT_DIR: portable(join(temp, 'output')),
            },
        });
        if (state === 'missing-commit') {
            assert.notEqual(result.status, 0, 'An unidentified source revision must fail the build');
            assert.doesNotMatch(result.stdout, /==> Building|source commit:.*unknown/);
        } else {
            assert.equal(result.status, 0, result.stderr);
            const dirty = state === 'clean' ? '' : ' (working tree has uncommitted changes)';
            assert.ok(result.stdout.includes(`source commit:         ${commit}${dirty}\n`), result.stdout);
            assert.ok(result.stdout.includes(`expanded WASM SHA-256: ${hash}`), result.stdout);
            assert.ok(result.stdout.includes(`publish this source as: composer-wasm-${hash.slice(0, 8)}`));
        }
        console.log(`PASS: ${state}`);
    } finally {
        // Only remove this invocation's newly allocated fixture directory.
        assert.equal(dirname(resolve(temp)), resolve(tmpdir()));
        rmSync(temp, { recursive: true, force: true });
    }
}
