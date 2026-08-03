#ifndef K4GEO_SIMPLEDRIFTCHAMBER_GEO_H
#define K4GEO_SIMPLEDRIFTCHAMBER_GEO_H

#include "SimG4Common/Geant4PreDigiTrackHit.h"

#include "DD4hep/Segmentations.h"
#include "DDSegmentation/Segmentation.h"

#include "G4THitsCollection.hh"
#include "G4VSensitiveDetector.hh"

namespace det {

class SimpleDriftChamber_geo : public G4VSensitiveDetector {
public:
  SimpleDriftChamber_geo(const std::string& aDetectorName,
                          const std::string& aReadoutName,
                          const dd4hep::Segmentation& aSeg);
  virtual ~SimpleDriftChamber_geo();

  virtual void Initialize(G4HCofThisEvent* aHitsCollections) final;
  virtual bool ProcessHits(G4Step* aStep, G4TouchableHistory*) final;

private:
  G4THitsCollection<k4::Geant4PreDigiTrackHit>* m_driftChamberCollection;
  dd4hep::Segmentation m_seg;

  double m_edepCut = 10 * CLHEP::eV;
  double m_stepLengthCut = 5 * CLHEP::micrometer;
};

}  // namespace det

#endif
