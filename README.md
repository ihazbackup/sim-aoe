# Where to put these stuffs?

Put these folder and files to ns-3/src/ndnSIM/examples (Overwrite existing)
* ndn-custom-apps (folder)
* ndn-custom-apps.cpp

Put these files to ns-3/src/ndnSIM/ndn-cxx/src (Overwrite existing)
* interest.cpp
* interest.hpp

Put these files to ns-3/src/ndnSIM/ndn-cxx/src/lp (Overwrite existing)
* tlv.hpp

# How to run

After you put / override every files, run these:
1. cd ns-3
2. ./waf configure --enable-examples
3. ./waf
4. ./waf run=ndn-custom-apps --vis

# Preliminary Experiment tasks

- [x] Customize Interest and Data packets with ATMT header
- [x] Node mobility and location-awareness
- [x] Broadcast strategy (May use Wifi module with limited range)
- [x] Customize node behavior when receiving interest or data packet
- [x] Visualization
- [ ] Custom forwarding strategy
- [ ] Metrics extraction
