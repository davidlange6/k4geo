#ifndef K4GEO_BIRKSLAWCALORIMETERSD_GEO_H
#define K4GEO_BIRKSLAWCALORIMETERSD_GEO_H

#include "SimG4Common/Geant4CaloHit.h"

#include "DD4hep/Segmentations.h"

#include "G4THitsCollection.hh"
#include "G4VSensitiveDetector.hh"

namespace det {

class BirksLawCalorimeterSD_geo : public G4VSensitiveDetector {
public:
  BirksLawCalorimeterSD_geo(const std::string& aDetectorName,
                             const std::string& aReadoutName,
                             const dd4hep::Segmentation& aSeg);
  virtual ~BirksLawCalorimeterSD_geo();

  virtual void Initialize(G4HCofThisEvent* aHitsCollections) final;
  virtual bool ProcessHits(G4Step* aStep, G4TouchableHistory*) final;

private:
  G4THitsCollection<k4::Geant4CaloHit>* m_calorimeterCollection;
  dd4hep::Segmentation m_seg;
  const std::string m_material;
  const double m_birk1;
  const double m_birk2;
};

}  // namespace det

#endif
