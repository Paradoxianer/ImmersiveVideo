//
// Panofelx Configurator
// copyright Matthias Lindner

#include <Roster.h>
#include <Window.h>
#include <View.h>
#include <MenuBar.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Path.h>
#include <Entry.h>
#include <TextView.h>
#include <TranslationUtils.h>
#include <ScrollView.h>
#include <File.h>

#include <string.h>
#include <stdio.h>
#include <math.h>

#include "Constants.h"
#include "ImmersiveVideoConfigurator.h"

// Application's signature

const char *APP_SIGNATURE				= "application/x-vnd.Be-ImmersiveVideoConfigurator";

BRect windowRect(50,50,250,400);

//
// ImmersiveVideoConfigurator::ImmersiveVideoConfigurator
//
// The constructor for the ImmersiveVideoConfigurator class.  This
// will create our window.
//
ImmersiveVideoConfigurator::ImmersiveVideoConfigurator(): BApplication(APP_SIGNATURE) {

	window=new ImmersiveVideoWindow(windowRect);
	
	outputWindow=NULL;
	openPanel=NULL;
	savePanel=NULL;
	exportPanel=NULL;
	lTable=NULL;
	original=NULL;
	neu=NULL;
}


//Komandozeilen argumente in refs verwandeln

void ImmersiveVideoConfigurator::ArgvReceived(int32 argc, char** argv)
{
	BMessage* message = 0;
	for (int32 i=1; i<argc; i++) {
		entry_ref ref;
		status_t err = get_ref_for_path(argv[i], &ref);
		if (err == B_OK) {
			if (! message) {
				message = new BMessage;
				message->what = B_REFS_RECEIVED;
			}
			message->AddRef("refs", &ref);
		}
	}
	if (message) {
		RefsReceived(message);
	}
}



//
// ImmersiveVideoConfigurator::MessageReceived
//
// Handle incoming messages.  
void ImmersiveVideoConfigurator::MessageReceived(BMessage *message) {
  /*printf("be_app Message Received\n");
  	message->PrintToStream();*/
	switch(message->what) {
		/*case PLUGIN_WIN_QUIT:
			pluginWin = NULL;
	 	break;*/
		case OUTPUT_WIN_QUIT:{
			outputWindow=NULL;
			break;
		}
		

		case MENU_FILE_OPEN:{
//			openPanel = new BFilePanel(B_OPEN_PANEL, be_app, NULL, 0, false, new BMessage(OPEN_FILE_PANEL), NULL, false, true);
			if (!openPanel){
				openPanel = new BFilePanel;
				openPanel->Window()->SetWorkspaces(B_CURRENT_WORKSPACE);
			}
			openPanel->Show();
//			delete openPanel;
			openPanel=NULL;
			break;
		}
		case MENU_FILE_CLOSE:{
			(new BMessenger(outputWindow,NULL,NULL))->SendMessage(B_QUIT_REQUESTED);
			break;
		}
		case FILE_SAVE:
		{
			ImmersiveVideoPlugin *pli;
			message->FindPointer("who",(void **)&pli);
			BMessage *archive =new BMessage();
			pli->Archive(archive,true);
//			archive->PrintToStream();
			Save(pli->GetName(),archive);
			break;
		}
		case MENU_FILE_SAVEAS:{
			if (!savePanel){
				savePanel = new BFilePanel(B_SAVE_PANEL);
				savePanel->Window()->SetWorkspaces(B_CURRENT_WORKSPACE);
			}
			savePanel->Show();			
			break;
		}
		case MENU_FILE_EXPORT:{
			if (!exportPanel){
				exportPanel = new TranslatorSavePanel("Bild Exportieren", &be_app_messenger, NULL, 0, false,new BMessage(EXPORT_IMAGE));
				exportPanel->Window()->SetWorkspaces(B_CURRENT_WORKSPACE);
			}
//			savePanel= new TranslatorSavePanel;
			exportPanel->Show();
			//delete savePanel;
			break;
		}
		case EXPORT_IMAGE:{
			Export(message);
			break;
		}
		case CALC_NEW_LOOK_UP_TABLE:{
				outputWindow->SetBitmap(original);
			/*	delete lTable;*/
				lTable=NULL;
				break;
		}
		case LOOK_UP_TABLE_CALCULATED:{
			printf("LookUpTableCalculated\n");
			ImmersiveVideoPlugin *plg;
	//		message->PrintToStream();
			if (original){
				delete neu;
				//**window->testInRequest=2;
				message->FindPointer("who",(void **)&plg);
				BRect neubounds=*(plg->outDim);
//				neubounds.right-=1;
//				neubounds.bottom+=1;
				neu=new BBitmap(neubounds,original->ColorSpace(),false);

				lTable=plg->GetLookUpTable();
				Calc();

			}
			break;
		}
		default:
			BApplication::MessageReceived(message);
			break;
	}
}

//
// ImmersiveVideoConfigurator::RefsReceived
//
// Verarbeited alle empfangenen Dateireferencen
//
void ImmersiveVideoConfigurator::RefsReceived(BMessage *message) {
  	//printf("be_app Refs Received\n");
//  	message->PrintToStream();
	uint32 type;
	int32 count;
	entry_ref ref;
	
	message->GetInfo("refs", &type, &count);
	if (type != B_REF_TYPE)
		return;
	
	for (int32 i = --count; i >= 0; --i) {
   		if (message->FindRef("refs", i, &ref) == B_OK) {
   			Open(&ref);
   		}
   	}
}

// laden eines Bildes
status_t ImmersiveVideoConfigurator::Open(entry_ref *ref) {
//	printf("Bin in open...\n");
	delete original;
	delete neu;
	original=NULL;
	neu=NULL;
	/*wenn Ausgabewindow schon erzeugt worde sperren um keine 
	sinnlosen Narichten und ungewünschte Effekte zu produzieren */
	if (!(outputWindow==NULL))
		outputWindow->Lock();
	//Bild laden ... easy dank Translatortechnick von BeOS ...	
	original = BTranslationUtils::GetBitmap(ref);
	//wenn ordentnlich geladen
	if (original)
	{
		//testen ob Ausgabefenster schon existier
		if (outputWindow==NULL)
		{
			//wenn nicht erzeugen und auch sperren, damit das Unlock ausgeglichen ist
			outputWindow=new OutputWin(window);
			outputWindow->Lock();
		}
		//geladenes Bild dem Ausgabefenster übergeben
		outputWindow->SetBitmap(original);
//		outputWindow->SetWorkspaces(B_CURRENT_WORKSPACE);
		//darstellen... wenn veborgen dann wider aufspringen usw....
		outputWindow->Show();
		//Lock ausbalancieren d.h. Fenster wieder freigeben 
		outputWindow->Unlock();
		//Abmessungen des Bildes holen
		BRect *originalBounds = new BRect(original->Bounds());
		/*kleine schönheits Korrektur um richtigen Werte zu erhalten
		(sonst würden bei 640x480 so was wie 639x479 da stehen in der GUI)
		*/
/*		originalBounds->bottom+=1.0;
		originalBounds->right+=1.0;*/
		// so und in der GUI anzeigen
		window->SetInputBounds(originalBounds);
		//ohne Fehler 
		return 1;
	}
	else
		//irgendein Fehler
		return -1;
	
}


void ImmersiveVideoConfigurator::Calc()
{
	//berechnen des neuen entzerrten bildes.
	//größe des Lookuptable Arrays berechnen
	lTableSize=(neu->Bounds().Width()+1)*(neu->Bounds().Height()+1);
	/*entsprechend der verwendeten Farbcodierung 
	(ARGB32 -> Int32 RGB16-> Int16 )
	dazughörige Methode aufrufen*/
	switch(sizeof(original->ColorSpace())) {
		case 4: 
			CalcInt32();
			break;
		case 2: 
			CalcInt16();
			break;
		case 1: 
			CalcInt8();
			break;

	}
	//entzerrtes Bild dem Ausgabefenster übergeben
	outputWindow->SetBitmap(neu);
}

void ImmersiveVideoConfigurator::CalcInt32()
{
	uint32 i,x,y;
	uint32 *neu_Bits=(uint32 *)neu->Bits();
	uint32 *original_Bits=(uint32 *)original->Bits();
	for (i=0;i<=lTableSize;i++)
	{
		x=(uint32)lTable[i].x;
		y=(uint32)lTable[i].y;
		neu_Bits[i]=original_Bits[x+(((original->BytesPerRow()/4))*y)];
	}
	outputWindow->SetBitmap(neu);
}

void ImmersiveVideoConfigurator::CalcInt16()
{
	uint32 i,x,y;
	uint16 *neu_Bits=(uint16 *)neu->Bits();
	uint16 *original_Bits=(uint16 *)original->Bits();
	for (i=0;i<=lTableSize;i++)
	{
		x=(uint32)lTable[i].x;
		y=(uint32)lTable[i].y;
		neu_Bits[i]=original_Bits[x+(((original->BytesPerRow()/2))*y)];
	}
}
void ImmersiveVideoConfigurator::CalcInt8()
{
	uint32 i,x,y;
	uint16 *neu_Bits=(uint16 *)neu->Bits();
	uint16 *original_Bits=(uint16 *)original->Bits();
	for (i=0;i<=lTableSize;i++)
	{
		x=(uint32)lTable[i].x;
		y=(uint32)lTable[i].y;
		neu_Bits[i]=original_Bits[x+(original->BytesPerRow()*y)];
	}
}


status_t ImmersiveVideoConfigurator::Save(const char* name,BMessage *archive) 
{
	//Configurationsdatei erzeugen
	BPath *configPath=new BPath();
	find_directory(B_COMMON_SETTINGS_DIRECTORY,configPath,false,NULL);
	configPath->Append("Immersive Video/config");
	configPath->Append(name);
	status_t returnCode;
	//bei gültigem Verweis eine Datei erzeugen, die existerenede Dateien überschreibt
	if (name!=NULL)
		saveFile=new BFile(configPath->Path(),B_READ_WRITE|B_CREATE_FILE);
	//testen ob Datei anlegen geklappt hat
	returnCode=saveFile->InitCheck();
	if (returnCode==B_OK)
	{
		//das Message Objekt welches zum Speichern dient erzeugen
	//	BMessage *saveMessage=new BMessage;
		/*Größe des Eingabebildes und des Ausgbabebildes in diesese Message Objekt
		serialisieren*/
//		saveMessage->AddRect("iDim",*window->GetInPutBounds());
//		saveMessage->AddRect("oDim",*window->GetOutPutBounds());
		//printf("sizeof(lTable)=%ld",sizeof(lTable));
		/*das es nicht gefunzt hat das Array ad hock in ein BMessage objekt zu übernehmen
		wird jeder Punkt unter dem selben Namen das saveMessageobjekt geschrieben
		das erzeugt auch ein Array*/
//		for (int i=0;i<lTableSize;i++)
//			saveMessage->AddPoint("lTable",lTable[i]);
		/*for (int i=0;i<=lTableSize;i++){
			saveMessage->AddFloat("x",lTable[i].x);
			saveMessage->AddFloat("y",lTable[i].y);
		}*/
//		saveMessage->AddData("lTable",B_POINT_TYPE,lTable[i],sizeof(lTable));
		//dann über Flattern einfach den Inhalt des Messageobjekts in die Dateschreiben
		returnCode=archive->Flatten(saveFile);
		
	}
	return returnCode;
}

//um mal Lookuptable laden zu können... bringts aber nicht wirklich
status_t ImmersiveVideoConfigurator::Load(entry_ref *ref)
{
	status_t returnCode;
	BFile *loadFile=new BFile(ref,B_READ_ONLY);
	returnCode=loadFile->InitCheck();
	if (returnCode==B_OK){
		BMessage *loadMessage=new BMessage;
/*		saveMessage->AddRect("iDim",window->GetInPutBounds());
		saveMessage->AddRect("oDim",window->GetOutPutBounds());
		saveMessage->AddData("lTable",B_POINT_TYPE,lTable,sizeof(BPoint),sizeof(lTable)/sizeof(BPoint));
		returnCode=saveMessage->Flatten(saveFile);*/
		loadMessage->Unflatten(loadFile);
	}
	return returnCode;
}



//export zum speichern entzerrter Bilder
status_t ImmersiveVideoConfigurator::Export(BMessage *message) {
	// Recover the necessary data from the message
	translator_id *id;
	uint32 format;
	ssize_t length = sizeof(translator_id);
	if (message->FindData("translator_id", B_RAW_TYPE, (const void **)&id, &length) != B_OK) return B_ERROR;
	if (message->FindInt32("translator_format", (int32 *)&format) != B_OK) return  B_ERROR;
	entry_ref dir;
	if (message->FindRef("directory", &dir) != B_OK) return  B_ERROR;
	BDirectory bdir(&dir);
	const char *name;
	if (message->FindString("name", &name) != B_OK) return  B_ERROR;
	if (name == NULL) return  B_ERROR;
	
	// Clobber any existing file or create a new one if it didn't exist
	BFile file(&bdir, name, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	if (file.InitCheck() != B_OK) {
		BAlert *alert = new BAlert(NULL, "Could not create file.", "OK");
		alert->Go();
		return B_ERROR;
	}
	BTranslatorRoster *roster = BTranslatorRoster::Default();
	BBitmapStream stream(neu);
	
	// If the id is no longer valid or the translator fails for any other
	// reason, catch it here
	if (roster->Translate(*id, &stream, NULL, &file, format) != B_OK) {
		BAlert *alert = new BAlert(NULL, "Could not save the image.", "OK");
		alert->Go();
	}
	
	// Reclaim the ownership of the bitmap, otherwise it would be deleted
	// when stream passes out of scope
	stream.DetachBitmap(&neu);
	return B_OK;
}





//
// main
//
// The main() function's only real job in a basic BeOS
// application is to create the BApplication object
// and run it.
//
int main(void) {
	ImmersiveVideoConfigurator theApp;		// The application object
	theApp.Run();
	return 0;
}



