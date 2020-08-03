#ifndef _GDMLDETECTORCONSTRUCTION_H_
#define _GDMLDETECTORCONSTRUCTION_H_

#include "DetectorConstruction.hh"
#include "globals.hh"


#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4SDManager.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4PSEnergyDeposit.hh"
#include "G4PSDoseDeposit.hh"
#include "G4VisAttributes.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4VSDFilter.hh"
#include "G4SDParticleFilter.hh"
#include "G4Scintillation.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4PhysicalConstants.hh"

#include "G4GDMLParser.hh"

using namespace std ;

class G4VPhysicalVolume;
class G4LogicalVolume;

//#include "B3DetectorConstruction.hh"
class GDMLDetectorConstruction : public DetectorConstruction
{
  public:
   GDMLDetectorConstruction(G4GDMLParser& parser, G4VPhysicalVolume *setWorld = 0) : DetectorConstruction()
   {
      World = setWorld;

	G4NistManager* man = G4NistManager::Instance();
  
  	G4bool isotopes = false;
  
  	G4Element*  H = man->FindOrBuildElement("H" , isotopes);
	G4Element*  C = man->FindOrBuildElement("C" , isotopes);

	G4Material* Vikuiti = new G4Material("Vikuiti", 1.07*g/cm3, 2);
	Vikuiti->AddElement(C,8);
	Vikuiti->AddElement(H,8);
	Vikuiti->GetIonisation()->SetBirksConstant(0.126*mm/MeV);

	G4Material* EJ208 = new G4Material("EJ208", 1.023*g/cm3, 2);
	EJ208->AddElement(C,9);
	EJ208->AddElement(H,10);
	EJ208->GetIonisation()->SetBirksConstant(0.1955*mm/MeV);
	
	const G4int nEntries = 6;

	G4double PhotonEnergy[nEntries] = {3.105*eV,2.95714*eV,2.855*eV,2.7*eV,2.5875*eV,2.388*eV}; //visible spectrum (400,420,435,460,480,520)nm
	
	G4double refractive_index_vk[nEntries] = {1.6,1.6,1.6,1.6,1.6,1.6};
	G4double att_length_vk[nEntries] = {400*cm,400*cm,400*cm,400*cm,400*cm,400*cm};

	G4double refractive_index_ej[nEntries] = {1.58,1.58,1.58,1.58,1.58,1.58};
	G4double att_length_ej[nEntries] = {400*cm,400*cm,400*cm,400*cm,400*cm,400*cm};

	G4double fast[nEntries] = {0.032258,0.258064,0.322581,0.225806,0.129032,0.032258};//{0.1,0.8,1,0.7,0.4,0.1}

	G4double reflectivity_vk[nEntries] = {0.9643,0.9680,0.9698,0.9743,0.9761,0.9798};
	G4double efficiency[nEntries] = {1.0,1.0,1.0,1.0,1.0,1.0};

	G4MaterialPropertiesTable* vkMPT = new G4MaterialPropertiesTable();
	G4MaterialPropertiesTable* scintMPT = new G4MaterialPropertiesTable();
	G4MaterialPropertiesTable* surfVKMPT = new G4MaterialPropertiesTable();
	G4MaterialPropertiesTable* surfEJMPT = new G4MaterialPropertiesTable();

	vkMPT->AddProperty("RINDEX", PhotonEnergy,refractive_index_vk,nEntries);
	vkMPT->AddProperty("ABSLENGTH", PhotonEnergy,att_length_vk,nEntries);

	scintMPT->AddProperty("RINDEX", PhotonEnergy,refractive_index_ej,nEntries);
	scintMPT->AddProperty("ABSLENGTH", PhotonEnergy,att_length_ej,nEntries);
	scintMPT->AddProperty("FASTCOMPONENT", PhotonEnergy, fast, nEntries);
  	scintMPT->AddConstProperty("SCINTILLATIONYIELD", 9200 / MeV);
  	scintMPT->AddConstProperty("RESOLUTIONSCALE", 1.0);
	scintMPT->AddConstProperty("FASTSCINTILLATIONRISETIME", 1.0 * ns);
 	scintMPT->AddConstProperty("FASTTIMECONSTANT", 3.3 * ns);
	scintMPT->AddConstProperty("YIELDRATIO", 1.0);

	EJ208->SetMaterialPropertiesTable(scintMPT);
	Vikuiti->SetMaterialPropertiesTable(vkMPT);
	
	surfVKMPT->AddProperty("REFLECTIVITY", PhotonEnergy, reflectivity_vk, nEntries);
	surfEJMPT->AddProperty("REFLECTIVITY", PhotonEnergy, reflectivity_vk, nEntries);
	surfVKMPT->AddProperty("EFFICIENCY", PhotonEnergy, efficiency, nEntries);
	surfEJMPT->AddProperty("EFFICIENCY", PhotonEnergy, efficiency, nEntries);
	surfVKMPT->AddProperty("RINDEX", PhotonEnergy,refractive_index_ej,nEntries);
	surfEJMPT->AddProperty("RINDEX", PhotonEnergy,refractive_index_ej,nEntries);



	G4OpticalSurface* surfVK = new G4OpticalSurface("surfVK");
	G4OpticalSurface* surfEJ = new G4OpticalSurface("surfEJ");

	surfVK->SetType(dielectric_dielectric);
	surfVK->SetModel(unified);
	surfVK->SetFinish(polishedfrontpainted);
	surfVK->SetMaterialPropertiesTable(surfVKMPT);

	surfEJ->SetType(dielectric_dielectric);
	surfEJ->SetModel(unified);
	surfEJ->SetFinish(polished);
	surfEJ->SetMaterialPropertiesTable(surfEJMPT);

	/* NEED TO ADD THESE - what are the values?
	surf->SetSigmaAlpha(sigma_alpha);
	MPT->AddProperty("SPECULARLOBECONSTANT",pp,specularlobe,NUM);
	MPT->AddProperty("SPECULARSPIKECONSTANT",pp,specularspike,NUM);
	MPT->AddProperty("BACKSCATTERCONSTANT",pp,backscatter,NUM);
	*/


	G4VisAttributes* red_col = new G4VisAttributes(G4Color(0.6,0.4,0.4,1));
	G4VisAttributes* blue_col = new G4VisAttributes(G4Color(0.4,0.4,0.6,1));

	G4LogicalVolume* EJvol;
	G4VPhysicalVolume* EJvol_P;
	G4LogicalSkinSurface* EJsurf;
	G4String a;
	for (G4int i = 1; i <= 48; i++) {
          //char a[100];
	  a = "S_EJ208(";
	  a = a + to_string(i);
	  a = a + ")";
          //sprintf(a,"%d", i);
          //strcat(a,")");
          EJvol = parser.GetVolume(a);
	  EJvol->SetMaterial(EJ208);
	  EJvol->SetVisAttributes (red_col);
	  EJsurf = new G4LogicalSkinSurface("surfEJ_L",EJvol, surfEJ); 	
        }

	G4LogicalVolume* VKvol;
	G4VPhysicalVolume* VKvol_P;
	G4LogicalSkinSurface* VKsurf;
	for (G4int i = 1; i <= 48; i++) {
          //char a[100];
	  a = "S_Vikuiti(";
	  a = a + to_string(i);
	  a = a + ")";
          //sprintf(a,"%d", i);
          //strcat(a,")");
          VKvol = parser.GetVolume(a);
	  VKvol->SetMaterial(Vikuiti);
	  VKvol->SetVisAttributes (blue_col);
	  VKsurf = new G4LogicalSkinSurface("surfVK_L",VKvol, surfVK);	
        }
	
	/*G4Tubs* testP = new G4Tubs("Crystal",0*cm,10*cm, 5*cm,0,2*pi);
	G4LogicalVolume* lc = new G4LogicalVolume(testP,EJ208,"Crystal");
	G4ThreeVector pos = G4ThreeVector(0*cm,0*cm,0*cm);
	G4VPhysicalVolume* pC = new G4PVPlacement(0,pos,"Crystal", lc, World, false,0); //test crystal

	G4Tubs* testP2 = new G4Tubs("Crystal2",4*cm,10*cm, 5*cm,0,2*pi);
	G4LogicalVolume* lc2 = new G4LogicalVolume(testP2,Vikuiti,"Crystal2");
	G4ThreeVector pos2 = G4ThreeVector(0*cm,8*cm,8*cm);
	G4VPhysicalVolume* pC2 = new G4PVPlacement(0,pos2,"Crystal2", lc2, World, false,0);*/
   }

   G4VPhysicalVolume *Construct()
   {
	
     return World;
   }

   virtual ~GDMLDetectorConstruction() {};

  public:
    void ConstructSDandField()
    {
        G4MultiFunctionalDetector* patient = new G4MultiFunctionalDetector("patient");
        G4SDManager::GetSDMpointer()->AddNewDetector(patient);
        G4VPrimitiveScorer* sec1 = new G4PSDoseDeposit("dose");
        patient->RegisterPrimitive(sec1);

        /*for (G4int i = 0; i < 109; i++) {
          char a[100];
          sprintf(a,"%d", i);
          strcat(a,"_EmphaticLV");
          SetSensitiveDetector(a,patient);
        }*/
        SetSensitiveDetector("world_volume",patient);

    }


  private:
    G4VPhysicalVolume *World;
    void DefineMaterials();
    G4bool  fCheckOverlaps;

};

#endif
