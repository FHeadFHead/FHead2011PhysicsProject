#include "DataFormats/HcalDigi/interface/CastorDataFrame.h"

CastorDataFrame::CastorDataFrame() : id_(0), 
			     size_(0),
			     hcalPresamples_(0)
{
}

CastorDataFrame::CastorDataFrame(const HcalCastorDetId& id) : 
  id_(id), 
  size_(0),
  hcalPresamples_(0)
{
  // TODO : test id for CASTOR
}
  
void CastorDataFrame::setSize(int size) {
  if (size>MAXSAMPLES) size_=MAXSAMPLES;
  else if (size<=0) size_=0;
  else size_=size;
}
void CastorDataFrame::setPresamples(int ps) {
  hcalPresamples_=ps;
}
//void CastorDataFrame::setReadoutIds(const HcalElectronicsId& eid) {
//  electronicsId_=eid;
//}

bool CastorDataFrame::validate(int firstSample, int nSamples) const {
  int capid=-1;
  bool ok=true;
  for (int i=0; ok && i<nSamples && i+firstSample<size_; i++) {
    if (data_[i+firstSample].er() || !data_[i+firstSample].dv()) ok=false;
    if (i==0) capid=data_[i+firstSample].capid();
    if (capid!=data_[i+firstSample].capid()) ok=false;
    capid=(capid+1)%4;
  }
  return ok;
}

std::ostream& operator<<(std::ostream& s, const CastorDataFrame& digi) {
  s << digi.id() << " " << digi.size() << " samples  " << digi.presamples() << " presamples " << std::endl;
  for (int i=0; i<digi.size(); i++) 
    s << "  " << digi.sample(i) << std::endl;
  return s;
}
  

