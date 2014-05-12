
#include "cellstate.hpp"

#include "esmreader.hpp"
#include "esmwriter.hpp"

void ESM::CellState::load (ESMReader &esm)
{
    mWaterLevel = 0;
    esm.getHNOT (mWaterLevel, "WLVL");

    mHasFogOfWar = false;
    esm.getHNOT (mHasFogOfWar, "HFOW");
}

void ESM::CellState::save (ESMWriter &esm) const
{
    if (!mId.mPaged)
        esm.writeHNT ("WLVL", mWaterLevel);

    esm.writeHNT("HFOW", mHasFogOfWar);
}
