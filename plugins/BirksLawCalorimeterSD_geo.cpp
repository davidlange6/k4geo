#include "BirksLawCalorimeterSD_geo.h"

#include "DD4hep/Detector.h"
#include "DDG4/Geant4Mapping.h"
#include "DDG4/Geant4VolumeManager.h"
#include "DDG4/Defs.h"
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

BirksLawCalorimeterSD_geo::BirksLawCalorimeterSD_geo(const std::string& aDetectorName,
                                                     const std::string& aReadoutName,
                                                     const dd4hep::Segmentation& aSeg)
    : G4VSensitiveDetector(aDetectorName),
      m_calorimeterCollection(nullptr),
      m_seg(aSeg),
      m_material("Polystyrene"),
      m_birk1(0.0130 * CLHEP::g / (CLHEP::MeV * CLHEP::cm2)),
      m_birk2(9.6e-6 * CLHEP::g / (CLHEP::MeV * CLHEP::cm2) * CLHEP::g / (CLHEP::MeV * CLHEP::cm2)) {
  collectionName.insert(aReadoutName);
}

BirksLawCalorimeterSD_geo::~BirksLawCalorimeterSD_geo() {}

void BirksLawCalorimeterSD_geo::Initialize(G4HCofThisEvent* aHitsCollections) {
  m_calorimeterCollection =
      new G4THitsCollection<k4::Geant4CaloHit>(SensitiveDetectorName, collectionName[0]);
  aHitsCollections->AddHitsCollection(G4SDManager::GetSDMpointer()->GetCollectionID(m_calorimeterCollection),
                                      m_calorimeterCollection);
}

bool BirksLawCalorimeterSD_geo::ProcessHits(G4Step* aStep, G4TouchableHistory*) {
  G4double edep = aStep->GetTotalEnergyDeposit();
  if (edep == 0.) return false;

  G4double response = 0.;
  G4Material* material = aStep->GetPreStepPoint()->GetMaterial();
  G4double charge = aStep->GetPreStepPoint()->GetCharge();

  if ((charge != 0.) && (m_material.compare(material->GetName()) == 0)) {
    G4double rkb = m_birk1;
    if (std::fabs(charge) > 1.0) rkb *= 7.2 / 12.6;

    if (aStep->GetStepLength() != 0) {
      G4double dedx = edep / (aStep->GetStepLength()) / (material->GetDensity());
      response = edep / (1. + rkb * dedx + m_birk2 * dedx * dedx);
    } else {
      response = edep;
    }
  } else {
    response = edep;
  }
  edep = response;

  const G4Track* track = aStep->GetTrack();
  CLHEP::Hep3Vector prePos = aStep->GetPreStepPoint()->GetPosition();
  auto hit = new k4::Geant4CaloHit(
      track->GetTrackID(), (int)track->GetDefinition()->GetPDGEncoding(), edep, track->GetGlobalTime());
  hit->cellID = cellID(m_seg, *aStep);
  hit->position = prePos;
  hit->energyDeposit = edep;
  m_calorimeterCollection->insert(hit);
  return true;
}

}  // namespace det

namespace dd4hep {
namespace sim {
static G4VSensitiveDetector* create_birks_law_calorimeter_sd_geo(const std::string& aDetectorName,
                                                                  dd4hep::Detector& aLcdd) {
  std::string readoutName = aLcdd.sensitiveDetector(aDetectorName).readout().name();
  return new det::BirksLawCalorimeterSD_geo(
      aDetectorName, readoutName, aLcdd.sensitiveDetector(aDetectorName).readout().segmentation());
}
}  // namespace sim
}  // namespace dd4hep

DECLARE_EXTERNAL_GEANT4SENSITIVEDETECTOR(BirksLawCalorimeterSD_geo, dd4hep::sim::create_birks_law_calorimeter_sd_geo)
