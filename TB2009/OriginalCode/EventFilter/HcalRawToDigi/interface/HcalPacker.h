#ifndef EVENTFILTER_HCALRAWTODIGI_HCALPACKER_H
#define EVENTFILTER_HCALRAWTODIGI_HCALPACKER_H 1

#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/FEDRawData/interface/FEDRawData.h"
#include "CondFormats/HcalObjects/interface/HcalElectronicsMap.h"

/** \class HcalPacker
  *  
  * $Date: 2007/02/19 04:05:40 $
  * $Revision: 1.1 $
  * \author J. Mans - Minnesota
  */
class HcalPacker {
public:
  struct Collections {
    Collections();
    const HBHEDigiCollection* hbhe;
    const HODigiCollection* hoCont;
    const HFDigiCollection* hfCont;
    const HcalCalibDigiCollection* calibCont;
    const ZDCDigiCollection* zdcCont;
    const HcalTrigPrimDigiCollection* tpCont;
  };

  void pack(int fedid, int dccnumber,
	    int nl1a, int orbitn, int bcn,
	    const Collections& inputs, 
	    const HcalElectronicsMap& emap,
	    FEDRawData& output);
private:
  int findSamples(const DetId& did, const Collections& inputs,
		  unsigned short* buffer, int &presamples);
};

#endif
