#ifndef WCSimWCHit_h
#define WCSimWCHit_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "G4LogicalVolume.hh"
#include "G4ios.hh"
// for sort, find, count_if
#include <algorithm>
//for less_equal, bind2nd,...
#include <functional>

using namespace std::placeholders;

#ifndef COMPOSE2
#define COMPOSE2 1
static std::function<bool(G4double)> compose2 (std::function<bool(bool, bool)> f, 
                                        std::function<bool(G4double)> f1, 
                                        std::function<bool(G4double)> f2){
    return [&](G4double i) { return f(f1(i), f2(i));};
}
#endif



class WCSimWCHit : public G4VHit
{
 public:
  
  WCSimWCHit();
  ~WCSimWCHit();
  WCSimWCHit(const WCSimWCHit&);
  const WCSimWCHit& operator=(const WCSimWCHit&);
  G4int operator==(const WCSimWCHit&) const;
  
  inline void* operator new(size_t);
  inline void  operator delete(void*);
  
  void Draw();
  void Print();
  
 public:
  
  void SetTubeID       (G4int tube)                 { tubeID = tube; };
  void SetTrackID      (G4int track)                { trackID = track; };
  void SetEdep         (G4double de)                { edep = de; };
  void SetPos          (G4ThreeVector xyz)          { pos = xyz; };
  void SetOrientation  (G4ThreeVector xyz)          { orient = xyz; };
  void SetRot          (G4RotationMatrix rotMatrix) { rot = rotMatrix; };
  void SetLogicalVolume(G4LogicalVolume* logV)      { pLogV = logV;}
  void AddParentID     (G4int primParentID) { primaryParentID.push_back(primParentID); }
  void AddPhotonStartTime (G4float photStartTime) { photonStartTime.push_back(photStartTime); }
  void AddPhotonStartPos  (const G4ThreeVector &photStartPos) { photonStartPos.push_back(photStartPos); }
  void AddPhotonEndPos  (const G4ThreeVector &photEndPos) { photonEndPos.push_back(photEndPos); }

  // This is temporarily used for the drawing scale
  void SetMaxPe(G4int number = 0)  {maxPe   = number;};

  void AddPe(G4float hitTime)  
  {
    // First increment the totalPe number
    totalPe++; 

    if (totalPe > maxPe) 
      maxPe = totalPe;

    time.push_back(hitTime);
  }
 
  G4int         GetTubeID()     { return tubeID; };
  G4int         GetTrackID()    { return trackID; };
  G4ThreeVector GetPos()        { return pos; };
  G4ThreeVector GetOrientation()        { return orient; };
  G4int         GetTotalPe()    { return totalPe;};
  G4float       GetTime(int i)  { return time[i];};
  G4int         GetParentID(int i) { return primaryParentID[i];};
  G4float       GetPhotonStartTime(int i) { return photonStartTime[i];};
  G4ThreeVector GetPhotonStartPos(int i) { return photonStartPos[i];};
  G4ThreeVector GetPhotonEndPos(int i) { return photonEndPos[i];};
  
  G4LogicalVolume* GetLogicalVolume() {return pLogV;};

  void SortHitTimes() {   sort(time.begin(),time.end()); }


  // low is the trigger time, up is trigger+950ns (end of event)
  G4float GetFirstHitTimeInGate(G4float low,G4float upevent)
  {
    G4float firsttime;
    std::vector<G4float>::iterator tfirst = time.begin();
    std::vector<G4float>::iterator tlast = time.end();
  
    std::vector<G4float>::iterator found = 
      std::find_if(tfirst,tlast,
		   compose2(std::logical_and<bool>(),
			    std::bind(std::greater_equal<G4double>(),    low, _1),
          std::bind(std::less_equal   <G4double>(),upevent, _1)
			    )
		   );
    if ( found != tlast ) {
      firsttime = *found; // first hit time
    }
    else {
      firsttime = -10000.; //error code.
    }
    //G4cout << "firsthit time " << firsttime << "\n";
    return firsttime;
  }


  // pmtgate  and evgate are durations, ie not absolute times

  G4int GetPeInGate(double low, double pmtgate,double evgate) {
    // M Fechner; april 2005
    // assumes that time has already been sorted
    std::vector<G4float>::iterator tfirst = time.begin();
    std::vector<G4float>::iterator tlast = time.end();
    // select min time
    G4float mintime = (pmtgate < evgate) ? pmtgate : evgate;
    
    // return number of hits in the time window...
    
    G4int number = std::count_if(tfirst,tlast,
				 compose2(std::logical_and<bool>(),
            std::bind(std::greater_equal<G4double>(),    low, _1),
            std::bind(std::less_equal   <G4double>(),mintime, _1)
					  )
				 );
    
    totalPeInGate = number;
    //    G4cout << "numer = " <<  number <<"\n";
    return number;
  }


 private:
  
  void HSVtoRGB(float& fR, float& fG, float& fB, float& fH, float& fS, float& fV);

  G4int            tubeID;
  G4int            trackID;
  G4double         edep;
  G4ThreeVector    pos;
  G4ThreeVector    orient;
  G4RotationMatrix rot;
  G4LogicalVolume* pLogV;

  // This is temporarily used for the drawing scale
  // Since its static *every* WChit sees the same value for this.

  static G4int     maxPe;

  G4int                 totalPe;
  std::vector<G4float>  time;
  std::vector<G4int>    primaryParentID;
  std::vector<G4float>  photonStartTime;
  std::vector<G4ThreeVector> photonStartPos;
  std::vector<G4ThreeVector> photonEndPos;
  G4int                 totalPeInGate;
};

typedef G4THitsCollection<WCSimWCHit> WCSimWCHitsCollection;

extern G4Allocator<WCSimWCHit> WCSimWCHitAllocator;

inline void* WCSimWCHit::operator new(size_t)
{
  void *aHit;
  aHit = (void *) WCSimWCHitAllocator.MallocSingle();
  return aHit;
}

inline void WCSimWCHit::operator delete(void *aHit)
{
  WCSimWCHitAllocator.FreeSingle((WCSimWCHit*) aHit);
}

#endif
