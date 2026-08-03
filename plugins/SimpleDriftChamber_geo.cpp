#include "SimpleDriftChamber_geo.h"

#include "DD4hep/Detector.h"
#include "DDG4/Geant4Mapping.h"
#include "DDG4/Geant4VolumeManager.h"
#include "DDG4/Factories.h"

#include "CLHEP/Vector/ThreeVector.h"

#include "G4SDManager.hh"

namespace {
static uint64_t cellID(const dd4hep::Segmentation& aSeg, const G4Step& aStep) {
  dd4hep::sim::Geant4VolumeManager volMgr = dd4hep::sim::Geant4Mapping::instance().volumeManager();
  dd4hep::VolumeID volID = volMgr.volumeID(aStep.GetPreStepPoint()->GetTouchable());
  if (aSeg.isValid()) {
    G4ThreeVector global = aStep.GetPreStepPoint()->GetPosition();
    G4ThreeVector local =
        aStep.GetPreStepPoint()->GetTouchable()->GetHistory()->GetTopTransform().TransformPoint(global);
    // G4 uses mm; DD4hep uses cm
    dd4hep::Position loc(local.x() * 0.1, local.y() * 0.1, local.z() * 0.1);
    dd4hep::Position glob(global.x() * 0.1, global.y() * 0.1, global.z() * 0.1);
    return aSeg.cellID(loc, glob, volID);
  }
  return volID;
}
}

namespace det {

SimpleDriftChamber_geo::SimpleDriftChamber_geo(const std::string& aDetectorName,
                                               const std::string& aReadoutName,
                                               const dd4hep::Segmentation& aSeg)
    : G4VSensitiveDetector(aDetectorName), m_driftChamberCollection(nullptr), m_seg(aSeg) {
  collectionName.insert(aReadoutName);
}

SimpleDriftChamber_geo::~SimpleDriftChamber_geo() {}

void SimpleDriftChamber_geo::Initialize(G4HCofThisEvent* aHitsCollections) {
  m_driftChamberCollection =
      new G4THitsCollection<k4::Geant4PreDigiTrackHit>(SensitiveDetectorName, collectionName[0]);
  aHitsCollections->AddHitsCollection(G4SDManager::GetSDMpointer()->GetCollectionID(m_driftChamberCollection),
                                      m_driftChamberCollection);
}

bool SimpleDriftChamber_geo::ProcessHits(G4Step* aStep, G4TouchableHistory*) {
  G4double edep = aStep->GetTotalEnergyDeposit();
  G4double stepLength = aStep->GetStepLength();

  if (edep < m_edepCut || stepLength < m_stepLengthCut) {
    return false;
  }

  const G4Track* track = aStep->GetTrack();
  CLHEP::Hep3Vector prePos = aStep->GetPreStepPoint()->GetPosition();
  CLHEP::Hep3Vector postPos = aStep->GetPostStepPoint()->GetPosition();

  auto hit = new k4::Geant4PreDigiTrackHit(
      track->GetTrackID(), (int)track->GetDefinition()->GetPDGEncoding(), edep, track->GetGlobalTime());

  hit->cellID = cellID(m_seg, *aStep);
  hit->energyDeposit = edep;
  hit->prePos = prePos;
  hit->postPos = postPos;
  m_driftChamberCollection->insert(hit);
  return true;
}

}  // namespace det

namespace dd4hep {
namespace sim {
static G4VSensitiveDetector* create_simple_driftchamber_geo(const std::string& aDetectorName,
                                                             dd4hep::Detector& aLcdd) {
  std::string readoutName = aLcdd.sensitiveDetector(aDetectorName).readout().name();
  return new det::SimpleDriftChamber_geo(
      aDetectorName, readoutName, aLcdd.sensitiveDetector(aDetectorName).readout().segmentation());
}
}  // namespace sim
}  // namespace dd4hep

DECLARE_EXTERNAL_GEANT4SENSITIVEDETECTOR(SimpleDriftChamber_geo, dd4hep::sim::create_simple_driftchamber_geo)
