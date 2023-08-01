//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __DIPOLEANTENNA_H
#define __DIPOLEANTENNA_H

#include "inet/common/INETDefs.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/AntennaBase.h"

using namespace inet;
using namespace physicallayer;

namespace gradys_simulations {
class DipoleAntennaCustom : public AntennaBase
{
  protected:
    virtual void initialize(int stage) override;

    class AntennaGain : public IAntennaGain
    {
      protected:
        Coord wireAxisDirection;
        m length;
        m lambda;
        double maxGain;
        double minGain;

      public:
        AntennaGain(const char *wireAxis, m length, m lambda, double maxGain, double minGain);
        virtual m getLength() const { return length; }
        virtual m getLambda() const { return lambda; }
        virtual double getMinGain() const override { return minGain; }
        virtual double getMaxGain() const override { return maxGain; }
        virtual double computeGain(const Quaternion& direction) const override;
    };

    Ptr<AntennaGain> gain;

  public:
    DipoleAntennaCustom();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual Ptr<const IAntennaGain> getGain() const override { return gain; }
};
}

#endif

