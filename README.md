== Where to put these stuffs? ==

Put these folder and files to ns-3/src/ndnSIM/examples (Overwrite existing)
- *Folder* ndn-custom-apps
- ndn-custom-apps.cpp

Put these files to ns-3/src/ndnSIM/ndn-cxx/src (Overwrite existing)
- interest.cpp
- interest.hpp

Put these files to ns-3/src/ndnSIM/ndn-cxx/src/lp (Overwrite existing)
- tlv.hpp

== How to run ==

1. cd ns-3
1. ./waf configure --enable-examples
2. ./waf
3. ./waf run=ndn-custom-apps --vis
