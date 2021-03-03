#include "inet/common/INETDefs.h"
#include "inet/mobility/single/RandomWaypointMobility.h"
#include "MobileNode.h"

#ifndef __INET_GOTOWPMOBILITY_H
#define __INET_GOTOWPMOBILITY_H

using namespace inet;

namespace projeto {

class GotoWaypointMobility : public RandomWaypointMobility
//class GotoWaypointMobility : public RandomWaypointMobility
{
  protected:
    virtual void setTargetPosition() override;
    virtual void setInitialPosition() override;

  public:
    GotoWaypointMobility();
    //MobileNode * myParentNode;

};

} // namespace inet

#endif // ifndef __INET_GOTOWPMOBILITY_H

