


#ifndef WCSimVisManager_h
#define WCSimVisManager_h 1
#include "G4VisManager.hh"


class WCSimVisManager: public G4VisManager {

public:

  WCSimVisManager ();

private:

  void RegisterGraphicsSystems ();

};


#endif
