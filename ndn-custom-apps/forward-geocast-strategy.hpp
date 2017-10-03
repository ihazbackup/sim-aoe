#ifndef FORWARD_GEOCAST_STRATEGY_HPP_
#define FORWARD_GEOCAST_STRATEGY_HPP_

#include <boost/random/mersenne_twister.hpp>
#include "face/face.hpp"
#include "fw/strategy.hpp"
#include "fw/algorithm.hpp"

namespace nfd {
namespace fw {

static double
GetDistance(double x1, double y1, double x2, double y2) 
{
  return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

static bool
IsInsideAoI(double posX, double posY, double aoiX, double aoiY, double radius)
{
  return posX <= aoiX + radius && posX >= aoiX - radius && 
  			posY <= aoiY + radius && posY >= aoiY - radius;
}

class ForwardGeocastStrategy : public Strategy {
public:
  ForwardGeocastStrategy(Forwarder& forwarder, const Name& name = STRATEGY_NAME);

  virtual ~ForwardGeocastStrategy() override;

  virtual void
  afterReceiveInterest(const Face& inFace, const Interest& interest,
                       const shared_ptr<pit::Entry>& pitEntry) override;

public:
  static const Name STRATEGY_NAME;
};

} // namespace fw
} // namespace nfd

#endif // FORWARD_GEOCAST_STRATEGY_HPP_
