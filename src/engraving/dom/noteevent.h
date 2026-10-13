/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MU_ENGRAVING_NOTEEVENT_H
#define MU_ENGRAVING_NOTEEVENT_H

#include <vector>
#include <algorithm>

#include "global/allocator.h"
#include "../infrastructure/eid.h"
#include "../types/fraction.h"
#include <limits>

namespace mu::engraving {
//---------------------------------------------------------
//    NoteEvent
//---------------------------------------------------------

class NoteEvent
{
public:
    struct TremoloTiming {
        Fraction emitterTick;
        Fraction emitterNominalSpan;
        Fraction emitterActualSpan;
        Fraction partnerActualSpan;
        Fraction ownerTick;
        Fraction ownerActualSpan;
        Fraction start;
        Fraction end; // Absolute source time; half-open before MIDI release conversion.
        int strokes = 0;
        int hooks = 0;
        int eventLength = 0;
        int eventOntime = 0;
        int eventOffset = 0;
        bool ownerHeld = false;
        bool captured = false;
        bool valid = false;
    };

    struct TremoloWrittenOwner {
        EID noteId = EID::invalid();
        EID chordId = EID::invalid();
        EID tremoloId = EID::invalid();
        bool captured = false;
        bool notePlayable = false;
        bool tremoloPlayable = false;
        TremoloTiming timing;

        bool isValid() const
        {
            return captured && noteId.isValid() && chordId.isValid() && tremoloId.isValid()
                   && timing.captured && timing.valid;
        }
    };

    constexpr static int NOTE_LENGTH = 1000;
    constexpr static double GHOST_VELOCITY_MULTIPLIER = 0.6;
    constexpr static double DEFAULT_VELOCITY_MULTIPLIER = 1.0;
    constexpr static int SLIDE_AMOUNT = 3;

    NoteEvent() {}
    NoteEvent(int a, int b, int c, double d = 1.0, double play = true, int offset = 0)
        : m_pitch(a), m_ontime(b), m_len(c), m_velocityMultiplier(d), m_play(play), m_offset(offset) {}

    int pitch() const { return m_pitch; }
    int ontime() const { return m_ontime; }
    int offtime() const { return m_ontime + m_len; }
    int len() const { return m_len; }
    double velocityMultiplier() const { return m_velocityMultiplier; }
    bool play() const { return m_play; }
    int offset() const { return m_offset; }
    bool slide() const { return m_slide; }
    void setPitch(int v) { m_pitch = v; }
    void setOntime(int v) { m_ontime = v; }
    void setLen(int v) { m_len = v; }
    void setVelocityMultiplier(double velocityMultiplier) { m_velocityMultiplier = velocityMultiplier; }
    void setOffset(int v) { m_offset = v; }
    void setSlide(bool slide) { m_slide = slide; }

    // Derived metadata is not serialized. Resolve these EIDs in the current score
    // before using copied or cached events; no source-score pointers are retained.
    const TremoloWrittenOwner& tremoloWrittenOwner() const { return m_tremoloWrittenOwner; }
    void setTremoloWrittenOwner(const EID& noteId, const EID& chordId, const EID& tremoloId,
                               bool notePlayable, bool tremoloPlayable)
    {
        m_tremoloWrittenOwner = {};
        m_tremoloWrittenOwner.noteId = noteId;
        m_tremoloWrittenOwner.chordId = chordId;
        m_tremoloWrittenOwner.tremoloId = tremoloId;
        m_tremoloWrittenOwner.captured = true;
        m_tremoloWrittenOwner.notePlayable = notePlayable;
        m_tremoloWrittenOwner.tremoloPlayable = tremoloPlayable;
    }

    void setTremoloTiming(TremoloTiming timing)
    {
        const Fraction maximum = Fraction::fromTicks(std::numeric_limits<int>::max());
        timing.captured = true;
        timing.eventLength = m_len;
        timing.eventOntime = m_ontime;
        timing.eventOffset = m_offset;
        timing.valid = timing.start.isValid() && timing.end.isValid()
                       && timing.start >= Fraction() && timing.end <= maximum && timing.end > timing.start
                       && timing.end.ticks() > timing.start.ticks() && m_len > 0;
        m_tremoloWrittenOwner.timing = timing;
    }

    void applyTremoloDurationGate(int previousLength, int gateTime)
    {
        auto timing = m_tremoloWrittenOwner.timing;
        if (!timing.valid || previousLength <= 0 || timing.eventLength <= 0 || gateTime <= 0
            || m_ontime != timing.eventOntime || m_offset != timing.eventOffset) {
            timing.valid = false;
            m_tremoloWrittenOwner.timing = timing;
            return;
        }
        timing.end = timing.start + (timing.end - timing.start)
                     * Fraction(previousLength, timing.eventLength) * Fraction(gateTime, 100);
        setTremoloTiming(timing);
    }

    bool operator==(const NoteEvent&) const;

private:
    int m_pitch = 0;     // relative pitch to note pitch
    int m_ontime = 0;    // one unit is 1/1000 of nominal note len
    int m_len = NOTE_LENGTH;       // one unit is 1/1000 of nominal note len
    double m_velocityMultiplier = DEFAULT_VELOCITY_MULTIPLIER; // can be used to lower sound (0-1)
    bool m_play = true; // when 'false', note event is used only for length calculation
    int m_offset = 0;
    bool m_slide = false; // event is slide or glissando
    TremoloWrittenOwner m_tremoloWrittenOwner;
};

//---------------------------------------------------------
//   NoteEventList
//---------------------------------------------------------

class NoteEventList : public std::vector<NoteEvent>
{
    OBJECT_ALLOCATOR(engraving, NoteEventList)
public:
    NoteEventList();

    int offtime()
    {
        return empty() ? 0 : std::max_element(cbegin(), cend(), [](const NoteEvent& n1, const NoteEvent& n2) {
            return n1.offtime() < n2.offtime();
        })->offtime();
    }
};
} // namespace mu::engraving
#endif
