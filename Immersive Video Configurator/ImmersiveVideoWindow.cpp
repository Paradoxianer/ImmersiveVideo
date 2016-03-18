//
// ImmersiveVideoWindow
//
// The Mainwindow of the Immersive Video Configurator
// 
//
// Written by: Matthias Lindner
//
/*
	Copyright 2004, Matthias Lindner   All Rights Reserved.
*/

#include <Application.h>
#include <Button.h>
#include <Entry.h>
#include <File.h>
#include <ListView.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <Messenger.h>
#include <Message.h>
#include <Path.h>
#include <Picture.h>
#include <PictureButton.h>
#include <Roster.h>
#include <StringView.h>
#include <String.h>
#include <ScrollView.h>
#include <TextControl.h>
#include <Window.h>
#include <View.h>



/*#include <string.h>*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ImmersiveVideoPlugin.h"
#include "ImmersiveVideoWindow.h"
#include "Constants.h"
#include "PluginItem.h"


ImmersiveVideoWindow::ImmersiveVideoWindow(BRect frame)
			: BWindow(frame, "Configurator", B_TITLED_WINDOW,0) {
	_Init();
	_InitWindow();
	Show();
}

ImmersiveVideoWindow::ImmersiveVideoWindow(BRect frame, entry_ref *ref)
			: BWindow(frame, "Configurator", B_TITLED_WINDOW,0) {
	_Init();
	_InitWindow();
	Show();
}

// Initialize variables
void ImmersiveVideoWindow::_Init(void) {
	relation=1;
	unitType=0;
	savemessage = NULL;			// No saved path yet
	//testInRequest=0;			//bisher noch keine Lookuptable berechnet
}

//Build up GUI eg MenuBar
void ImmersiveVideoWindow::_InitWindow(void) {
	BRect r;
	BMenu *menu;
	BMenuItem *item;
	
	//++++MenuBar++++
		r = Bounds();
		menubar = new BMenuBar(r, "menu_bar");

		// Add File menu to menu bar
	
		menu = new BMenu("File");
	//	menu->AddItem(new BMenuItem("New", new BMessage(MENU_FILE_NEW), 'N'));
		menu->AddItem(item=new BMenuItem("Open" B_UTF8_ELLIPSIS,new BMessage(MENU_FILE_OPEN), 'O'));
		item->SetTarget(be_app);
		menu->AddItem(item=new BMenuItem("Close", new BMessage(MENU_FILE_CLOSE), 'W'));
		item->SetTarget(be_app);
		menu->AddSeparatorItem();
		menu->AddItem(saveitem=new BMenuItem("Save", new BMessage(MENU_FILE_SAVE), 'S'));
//		saveitem->SetEnabled(false);
		//saveitem->SetTarget(be_app);
		//menu->AddItem(item=new BMenuItem("Save as" B_UTF8_ELLIPSIS,new BMessage(MENU_FILE_SAVEAS)));
		//item->SetTarget(be_app);
		menu->AddSeparatorItem();
		menu->AddItem(item=new BMenuItem("Export" B_UTF8_ELLIPSIS,new BMessage(MENU_FILE_EXPORT)));
		item->SetTarget(be_app);
		menu->AddSeparatorItem();
		menu->AddItem(item=new BMenuItem("Page Setup" B_UTF8_ELLIPSIS,new BMessage(MENU_FILE_PAGESETUP)));
		item->SetEnabled(false);
		item->SetTarget(be_app);
		menu->AddItem(item=new BMenuItem("Print" B_UTF8_ELLIPSIS,new BMessage(MENU_FILE_PRINT), 'P'));
		item->SetTarget(be_app);
		item->SetEnabled(false);
		menu->AddSeparatorItem();
		menu->AddItem(item=new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q'));
		item->SetTarget(be_app);
		menubar->AddItem(menu);
		
	
		// Add the Edit menu to the menu bar

		edit = new BMenu("Edit");
		edit->AddItem(item=new BMenuItem("Undo", new BMessage(B_UNDO), 'Z'));
//		edit->AddItem(item=new BMenuItem("Redo", new BMessage(B_REDO), 'Z'));

		edit->AddSeparatorItem();
		edit->AddItem(item=new BMenuItem("Cut", new BMessage(B_CUT), 'X'));
		edit->AddItem(item=new BMenuItem("Copy", new BMessage(B_COPY), 'C'));
		edit->AddItem(item=new BMenuItem("Paste", new BMessage(B_PASTE), 'V'));
		edit->AddSeparatorItem();
		edit->AddItem(item=new BMenuItem("Preferences", new BMessage(MENU_EDIT_PREFERENCES), 'A'));
		menubar->AddItem(edit);

		// Attach the menu bar to he window
	
		AddChild(menubar);
	//----MenuBar----

	//++++GUI++++
		//+++BackView+++
		BRect background= Bounds();
		background.top = menubar->Bounds().bottom + 1.0;
		BView *backView=new BView(background,"hintergrund",B_FOLLOW_ALL_SIDES, B_WILL_DRAW|B_PULSE_NEEDED);
		AddChild(backView);
		backView->SetViewColor(216,216,216);	
		//---BackView---
		
		//+++PluginListe+++
		BRect pluginListFrame = background;
		pluginListFrame.right -= B_V_SCROLL_BAR_WIDTH;
		pluginListFrame.bottom -= 180;
		//**nur notwendig wenn wir unten ein Scrollbar einführen wollen
	//	pluginListFrame.bottom -= B_V_SCROLL_BAR_WIDTH;
		pluginListFrame.InsetBy(2,2);
		listView = new BListView(pluginListFrame, "plugin_view", B_SINGLE_SELECTION_LIST,
				B_FOLLOW_ALL_SIDES, B_WILL_DRAW|B_PULSE_NEEDED);
		scrollView = new BScrollView("scroll_view", listView, B_FOLLOW_ALL_SIDES,  B_SUBPIXEL_PRECISE, false, true);
		backView->AddChild(scrollView);
		listView->MakeFocus(true);
		listView->SetSelectionMessage(new BMessage(PLUGINLIST_SELECTION_CHANGED));
		//---PluginListe---

		//+++Originalgrößen View+++
		BRect pictureSize = background;
		pictureSize.top=(pluginListFrame.bottom+5);
		pictureSize.bottom -=115;
		BBox *pictureSizeView=new BBox(pictureSize,"pictureView1",B_FOLLOW_BOTTOM|B_FOLLOW_LEFT_RIGHT);
		backView->AddChild(pictureSizeView);
		pictureSizeView->SetLabel(new BStringView(BRect(10,0,50,20),"Label","Originalgröße:"));
		BStringView *label1=new BStringView(BRect(5,20,45,35),"Label1","Width:", B_FOLLOW_BOTTOM|B_FOLLOW_LEFT , B_WILL_DRAW|B_PULSE_NEEDED);
		BStringView *unit1=new BStringView(BRect(pictureSize.Width()-75,20,pictureSize.Width()-60,35),"Unit1","px", B_FOLLOW_BOTTOM|B_FOLLOW_RIGHT , B_WILL_DRAW|B_PULSE_NEEDED);
		inputWidth=new BStringView(BRect(65,20,pictureSize.Width()-80,35),"iWidth","",B_FOLLOW_ALL_SIDES, B_WILL_DRAW|B_PULSE_NEEDED);


		BStringView *label2=new BStringView(BRect(5,40,45,55),"Label2","Height:", B_FOLLOW_BOTTOM|B_FOLLOW_LEFT, B_WILL_DRAW|B_PULSE_NEEDED);
		BStringView *unit2=new BStringView(BRect(pictureSize.Width()-75,40,pictureSize.Width()-60,55),"Unit2","px", B_FOLLOW_BOTTOM|B_FOLLOW_RIGHT , B_WILL_DRAW|B_PULSE_NEEDED);
		inputHeight=new BStringView(BRect(65,40,pictureSize.Width()-80,55),"iHeight","",B_FOLLOW_ALL_SIDES, B_WILL_DRAW|B_PULSE_NEEDED);
		pictureSizeView->AddChild(label1);
		pictureSizeView->AddChild(inputHeight);
		pictureSizeView->AddChild(unit1);
		inputHeight->SetViewColor(200,200,200,255);
		inputHeight->SetDrawingMode(B_OP_OVER);

//		(inputHeight->TextView())->MakeEditable(false);
		pictureSizeView->AddChild(label2);
		pictureSizeView->AddChild(inputWidth);
		pictureSizeView->AddChild(unit2);
		inputWidth->SetViewColor(200,200,200,255);
		inputWidth->SetDrawingMode(B_OP_OVER);

//		(inputWidth->TextView())->MakeEditable(false);
		//---Originalgrößen View---

		//+++Neue Größe View+++
		BRect newPictureSize = background;
		newPictureSize.top=(pictureSize.bottom+3);
		newPictureSize.bottom -=50;
		BBox *newPictureSizeView=new BBox(newPictureSize,"pictureView2",B_FOLLOW_BOTTOM|B_FOLLOW_LEFT_RIGHT);
		newPictureSizeView->SetLabel(new BStringView(BRect(10,0,50,20),"Label","Neue Größe:"));
		outputWidth=new BTextControl(BRect(5,15,newPictureSize.Width()-80,20),"oWidth","Width:","",new BMessage(OUTPUT_WIDTH_CHANGED),B_FOLLOW_ALL_SIDES, B_WILL_DRAW|B_PULSE_NEEDED);
		outputHeight=new BTextControl(BRect(5,35,newPictureSize.Width()-80,20),"oHeight","Height:","",new BMessage(OUTPUT_HEIGHT_CHANGED),B_FOLLOW_ALL_SIDES, B_WILL_DRAW|B_PULSE_NEEDED);


		//Die Maseinheiten auswahl...
		BMenu *messureMenu=new BMenu("px",B_ITEMS_IN_COLUMN);
		messureMenu->SetRadioMode(true);
		messureMenu->SetLabelFromMarked(true);
		messureMenu->AddItem(item=new BMenuItem("px",new BMessage(MESSURE_MENU_PIXEL)));
		item->SetMarked(true);
		messureMenu->AddItem(item=new BMenuItem("%",new BMessage(MESSURE_MENU_PERCENT)));
		BMenuField *messureKind=new BMenuField(BRect(newPictureSize.Width()-40,25,newPictureSize.Width()-5,40),"messureKind",NULL,messureMenu,B_FOLLOW_BOTTOM|B_FOLLOW_RIGHT);
		
		BRect rect=BRect(newPictureSize.Width()-75,20,newPictureSize.Width()-45,50);
		BView *tmpView = new BView(rect, "temp", B_FOLLOW_NONE, B_WILL_DRAW );
 			
		// der Verknüpfungsknop
		AddChild(tmpView);
		//create on picture
		rgb_color back=backView->ViewColor();
		//back.alpha=255;
	   	BPicture *on;
 		tmpView->BeginPicture(new BPicture); 
			tmpView->SetHighColor(back);
 			tmpView->FillRect(tmpView->Bounds());
			tmpView->BeginLineArray(7);
				tmpView->AddLine(BPoint(0,3),BPoint(15,3),black);
				tmpView->AddLine(BPoint(0,3),BPoint(5,0),black);
				tmpView->AddLine(BPoint(0,3),BPoint(5,6),black);
				tmpView->AddLine(BPoint(15,3),BPoint(15,27),black);
				tmpView->AddLine(BPoint(0,27),BPoint(15,27),black);
				tmpView->AddLine(BPoint(0,27),BPoint(5,24),black);
				tmpView->AddLine(BPoint(0,27),BPoint(5,30),black);
			tmpView->EndLineArray();
 			on = tmpView->EndPicture();
   			//create off picture
   			BPicture *off;
   			tmpView->BeginPicture(new BPicture); 
			tmpView->SetHighColor(back);
			tmpView->FillRect(tmpView->Bounds());
			tmpView->BeginLineArray(8);
				tmpView->AddLine(BPoint(0,3),BPoint(15,3),black);
				tmpView->AddLine(BPoint(0,3),BPoint(5,0),black);
				tmpView->AddLine(BPoint(0,3),BPoint(5,6),black);
				tmpView->AddLine(BPoint(15,3),BPoint(15,10),black);
				tmpView->AddLine(BPoint(15,20),BPoint(15,27),black);
				tmpView->AddLine(BPoint(0,27),BPoint(15,27),black);
				tmpView->AddLine(BPoint(0,27),BPoint(5,24),black);
				tmpView->AddLine(BPoint(0,27),BPoint(5,30),black);
			tmpView->EndLineArray();
			tmpView->SetPenSize(1.5);
			tmpView->SetHighColor(red);
			tmpView->StrokeArc(BPoint(15,7),4,2,180,180);
			tmpView->StrokeArc(BPoint(15,23),4,2,0,180);
   			off = tmpView->EndPicture();
   			//get rid of tmpView

		RemoveChild(tmpView);
		delete tmpView;
		depent = new BPictureButton(rect,"concate", on, off, NULL,B_TWO_STATE_BUTTON,B_FOLLOW_BOTTOM|B_FOLLOW_RIGHT);
		
		newPictureSizeView->AddChild(outputHeight);
		newPictureSizeView->AddChild(outputWidth);
		newPictureSizeView->AddChild(depent);
		newPictureSizeView->AddChild(messureKind);
		backView->AddChild(newPictureSizeView);
		
//		BButton	*testButton=new BButton(BRect(10,Bounds().Height()-45,70,Bounds().Height()-25),"TestButton","Test",new BMessage(TEST_REQUEST),B_FOLLOW_BOTTOM);
		BButton	*testButton=new BButton(BRect(Bounds().Width()-60,Bounds().Height()-45,Bounds().Width()-7,Bounds().Height()-25),"TestButton","Berechnen",new BMessage(TEST_REQUEST),B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM);
//		BButton	*speichernButton=new BButton(BRect(Bounds().Width()-70,Bounds().Height()-45,Bounds().Width()-10,Bounds().Height()-25),"SaveButton","Save",new BMessage(TEST_REQUEST),B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM);
		backView->AddChild(testButton);
//		backView->AddChild(speichernButton);
		
		//---Neue Größe View---

	pluginWindow=NULL;
	

	LoadPlugins();

}

void ImmersiveVideoWindow::LoadPlugins(void) {
	image_id		addonId;
   	status_t 		err = B_NO_ERROR; 
	ImmersiveVideoPlugin* aktPlugin=NULL;
	ImmersiveVideoPlugin* (*NewImmersiveVideoPlugin)(image_id);
	app_info		info;
	BFile			*file;
	BPath			path;
	BPath			*configPath = new BPath();
	BPath			*pluginPath	= new BPath();
	BEntry			*entry	=new BEntry();

	find_directory(B_COMMON_SETTINGS_DIRECTORY,pluginPath,false,NULL);
	pluginPath->Append("Immersive Video/plugins");
	/*pfad von der Anwendung besorgen, von der aus ein 
	ImmersiveVideoWindow aufgerufen wurde*/	
/*	be_app->GetAppInfo(&info); 
    BEntry entry(&info.ref); 
    entry.GetPath(&path); 
    path.GetParent(&path);
    //in das Unterverzeichniss "Plugins wechseln"
	path.Append("plugins");*/
	BDirectory dir(pluginPath->Path());
	printf("Laden der Plugins..\n");
	/*config Pfad holen*/
	find_directory(B_COMMON_SETTINGS_DIRECTORY,configPath,false,NULL);
	configPath->Append("Immersive Video/config");
	BDirectory configDir(configPath->Path());
	/*alle Dateien in diesem Verzeichniss durchlaufen 
	und jede Datei überprüfen ob sie ein gültiges Plugin ist.
	?muss ich vorher testen ob der Pfadgültig ist ?*/
	while( err == B_NO_ERROR )
	{
		// nächsten Verweis auf eine Datei auslesen
		err = dir.GetNextEntry(entry, TRUE );			
		//testen ob es ein gültiger Verweis ist
		if( entry->InitCheck() != B_NO_ERROR )
		{
			break;
		}
		//einen Pfad(+Dateiname) von dem aktuellen Verweis erstellen lassen ->Path
		if( entry->GetPath(&path) != B_NO_ERROR )
		{
			//printf( "entry.GetPath failed\n" );
		}
		else
		{
			//Versuch die über path angegebene Datei als Addon zu laden
			addonId = load_add_on( path.Path() );
			if( addonId < 0 )
			{
				//Wenns schief ging, kein Poblem, dann wars irgendeine andere Datei
				//printf( "load_add_on( %s ) failed\n", path.Path() );
				
			}
			else
			{
				//wenns geklappt hatt dann kleine Naricht auf stdout
				printf( "\t%s\t\tloaded\n", path.Leaf());
				/*überprüfen ob es ein Addon war, welches unsere Schnittstelle entspricht
				Schnittstelle heist hier NewImmersiveVideoPlugin und es muss mit dem Protypen
				(void **)NewImmersiveVideoPlugin gekennzeichnet*/
				if( get_image_symbol( addonId, 
									"NewImmersiveVideoPlugin", 
									B_SYMBOL_TYPE_TEXT, 
									(void **)&NewImmersiveVideoPlugin) )
				{
					// wenn wir ein ungülitges Plugin geladen hatten, wieder wegwerfen ;-)
					//printf( "get_image_symbol( NewImmersiveVideoPlugin ) failed\n" );
					unload_add_on(addonId );
				}
				else
				{
					//ansonsten von dem geleadenen Plugin mit der addonID ein gültiges Objekt erzeugen
					aktPlugin = (*NewImmersiveVideoPlugin)( addonId );
					if( !aktPlugin )
					{
						printf( "failed to create a new plugin\n" );
					}
					else
					{
						file=new BFile(&configDir,aktPlugin->GetName(),B_READ_ONLY);
						if (file->InitCheck()==B_OK)
						{
							//BMessage laden welches alle Einstellungen enhält
							BMessage *archive=new BMessage();
							archive->Unflatten(file);
							//daraus das Plugin alles bauen lassen, was es benötigt
							aktPlugin->Init(archive);
							delete archive;
							archive=NULL;
						}
						else
						{
							//wenn keine Configdatei gefunden wurde, wird das Plugin "pur" initialisiert
							aktPlugin->Init();
						}
				//		pluginWindow = new PluginWin(this,aktPlugin);
						//aktuelles Plugin der Liste (in diesm Fall der Grafischen Liste) als Eintrag hinzufügen
						listView->AddItem(new PluginItem(aktPlugin));
					}
				}
			}	
		}
	}
	listView->Select(0);
}


ImmersiveVideoWindow::~ImmersiveVideoWindow() {
	if (savemessage) {
		delete savemessage;
	}
	/*delete	menubar;
	delete	saveitem;
	delete	edit;
	delete	listView;
	delete	scrollView;
	delete	inputHeight;
	delete	inputWidth;
	delete	outputHeight;
	delete	outputWidth;		
	delete	depent;*/
}


void ImmersiveVideoWindow::MessageReceived(BMessage *message) {
/*	printf("PWindow Message recived\n");
	message->PrintToStream();*/
	int tmp;
	//unterschsceiden welcher Art Message hier angekommen ist
	switch(message->what) {
		case MENU_FILE_NEW:
			{
				//**What shoud we do on new?
					}
			break;
		case MENU_FILE_QUIT:
			//der Anwendung bescheid sagen, dass der Benutzer die Anwendung beenden will.
			be_app_messenger.SendMessage(B_QUIT_REQUESTED);
			break;
			
		case MESSURE_MENU_PERCENT:
			//die Einheit wude auf Prozent umgestellt
		{
			/*Eigene Methoden aufrufen die anhand der GUI Eingabefelder 
			Rechtecke mit entsprechenden Größen zu konstruieren*/
			BRect *in=GetInPutBounds();
			BRect *out=GetOutPutBounds();
			
			BString tmpString;
			/*Das Fenster Locken, damit Änderungen nicht sofort wieder eine Message erzeugen und
			das Programm in eine Endlosschleife gerät*/
			Lock();
			//umrechnen und zurück in die GUI schreiben
			tmpString << (float)(out->Width()/in->Width()*100.0);
			outputWidth->SetText(tmpString.String());
			tmpString="";
			tmpString << (float)(out->Height()/in->Height()*100.0);
			outputHeight->SetText(tmpString.String());
			//!!! Und wieder das Fenster freigeben damit es nicht blockiert
			Unlock();
			//welcher Typ gewählt wurde ... irgendwann noch durch Enum Constanten ersetzen
			unitType=1;
			delete in;
			delete out;
			//printf("UnitType=%d\n",unitType);
			break;
		}
		case MESSURE_MENU_PIXEL:
		{
			/*Eigene Methoden aufrufen die anhand der GUI Eingabefelder 
			Rechtecke mit entsprechenden Größen zu konstruieren*/
			BRect *in=GetInPutBounds();
			BRect *out=GetOutPutBounds();
			//rest wie oben
			BString tmpString;
			Lock();
			tmpString << (out->Width()+1);
			outputWidth->SetText(tmpString.String());
			tmpString="";
			tmpString << (out->Height()+1);
			outputHeight->SetText(tmpString.String());
			unitType=0;
		
			Unlock();
			delete in;
			delete out;
		//	printf("UnitType=%d\n",unitType);			
			break;
		}
		case PLUGINLIST_SELECTION_CHANGED:
			/*Methode aufrufen, die die nötigen Schritte unternimmt,
			 wenn der Nutzer ein Plugin aus der Liste auswählt*/
			SelectionChanged(message);
			break;
		case OUTPUT_WIDTH_CHANGED:
		{
			//Das Textfeld für die Ausgabegröße wurde verändert
			BString tmpString; 
			//Feldinhalt in eine Zahl umwandeln
			tmp =atoi(outputWidth->Text());
			//diese wieder in einen String
			tmpString << tmp;
			/*und dann den erzeugten String in das Textfeld zurück schreiben.
			Dadurch lassen sich Fehleingaben elegant und ohne nervendes Popupwindow 
			"löschen" und dem Nutzer wird durch nullen seiner Eingabe ein Fehler angezeigt ;-)*/
			outputWidth->SetText(tmpString.String());
//			printf("UnitType=%d\n",unitType);			
//			printf("Width=%d \n",tmp);
			//testen ob mit Picturebutton die Relation ein oder ausgeschaltet ist
			if (!depent->Value())
			{
				tmpString="";
				//Unit Pix .. Umrechnen
				if (!unitType) 
					tmpString << tmp/relation;
				//Unit Prozent einfach das selbe übernehmenb
				else
					tmpString << tmp;
				outputHeight->SetText(tmpString.String());
			}
			break;
		}
		case OUTPUT_HEIGHT_CHANGED:
		{
			// siehe case OUTPUT_WIDTH_CHANGED
			BString tmpString; 
			tmp =atoi(outputHeight->Text())+1;
			tmpString<<tmp;
			outputHeight->SetText(tmpString.String());
			//printf("UnitType=%d\n",unitType);			

//			printf("Height=%d \n",tmp);
			if (!depent->Value())
			{
				tmpString="";
				if (!unitType){
					tmpString << tmp*relation;
				}
				else{
					tmpString << tmp;
				}
				outputWidth->SetText(tmpString.String());
			}
			break;
		}
		case TEST_REQUEST:
		{
			//**testInRequest=1;
			//Button Berechen wurde gedrückt
			//printf("TestRequest\n");
			/* der Hauptandwendung bescheid geben dass die Lookuptable neu berechnet werden soll
				damit das Bild vorübergehend wieder auf Original "umgestellt wird" und die alte
				Lookuptbale gelöscht wirdpo(Speicher efizzenz)
			*/
			be_app->MessageReceived(new BMessage(CALC_NEW_LOOK_UP_TABLE));
			// rausbekommen mit welchem Plugin das ganze geschehen soll
			int select=listView->CurrentSelection(0);
			if (select>=0)
			{
				//Das Plugin "herausholen" aus der Lister
				PluginItem* pli=(PluginItem*)(listView->ItemAt(select)); 
				if (pli!=NULL)
				{
					//nötigen Werte an das Plugin übergeben
					pli->GetPlugin()->SetInputDimension(GetInPutBounds());
					pli->GetPlugin()->SetOutputDimension(GetOutPutBounds());
					/*Für die eigentliche Berechnung in einen eigenen Thread anlegen damit das Fenster
					und die Anwendung nicht blockiert werden*/
					thread_id pluginSam = spawn_thread((thread_func)(pli->GetPlugin()->RunCalculation), "PluginSam", B_NORMAL_PRIORITY, pli->GetPlugin());
					if (pluginSam >= 0)
						//thread starten, wenn Anlegen erfolgreiche
						resume_thread(pluginSam);
					else 
					{
						//ansonsten halt ohne extra thread einfach so starten
						pli->GetPlugin()->Run();
					}
				}
				else
				{
				
				}
			}
			else
			{
				//**Alert KeinPlugin selected...
			}
			break;
		}
		case B_SIMPLE_DATA:
		{
			/*diese Art Message wird bei Drag and Drop erzeugt wir lassen das ganze von 
			der hauptanwenwdung bearbeiten*/
			be_app->RefsReceived(message);
			break;
		}
		case MENU_FILE_SAVE_ENABLE:
		{
			//Auftrag bekommen den Menupunkt Save ein zu schalten ... machen wir auch
		//	saveitem->SetEnabled(true);
		}
		case MENU_FILE_SAVE:
		{
				PRINTF("MENU_FILE_SAVE\n");	
				int select=listView->CurrentSelection(0);
				if (select>=0)
				{
					BMessage *msg=new BMessage(FILE_SAVE);
					PluginItem* pli=(PluginItem*)(listView->ItemAt(select));
					msg->AddPointer("who",pli->GetPlugin());
					be_app_messenger.SendMessage(msg);
				}
		}
		default:
			//alles andere soll standartmäsig behandelt werden
		 	BWindow::MessageReceived(message);
		 	break;
	}
}



void ImmersiveVideoWindow::SelectionChanged(BMessage *message) 
{
	//momentan ausgewähltes Elment der Liste herausfinden
	//**testInRequest=0;
	int select=listView->CurrentSelection(0);
	if (select>=0)
	{
		//Plugin aus dem Listeneintrag herausholen
		PluginItem* pli=(PluginItem*)(listView->ItemAt(select));
		/*Meueinträge wie Cut Copy und Paste auf Das Plugin umlenken, damit es 
		in seiner GUI damit umgmehen kann*/
		edit->SetTargetForItems(pli->GetPlugin());
		/*vom Plugin entsprechend seiner Möglickeiten Cut,Copy,Paste und Undo sowie Redo
		aktiviern oder deativieren lassen*/
		edit->FindItem("Undo")->SetEnabled(pli->GetPlugin()->UnAndRedoPossible());
		//edit->FindItem("Redo")->SetEnabled(pli->GetPlugin()->UnAndRedoPossible());
		edit->FindItem("Cut")->SetEnabled(pli->GetPlugin()->ToolsPossible());
		edit->FindItem("Copy")->SetEnabled(pli->GetPlugin()->ToolsPossible());
		edit->FindItem("Paste")->SetEnabled(pli->GetPlugin()->ToolsPossible());
		if (pli!=NULL)
		{
			if (pluginWindow==NULL)
			{
				/*wenn noch kein PluginWindow erzeugt wurde eine neues PluginWindow
				mit dem aktuellen Plugin als GUI lieferer erzeugen*/			
				pluginWindow=new PluginWin(this,pli->GetPlugin());
			}
			else
			{
				/*dem Pluginfenster das aktuelle Plugin übergeben,
				 damit es die GUI davon bekommen kann*/
				pluginWindow->SetPlugin(pli->GetPlugin());
			}
		}
	}
}

//
bool ImmersiveVideoWindow::QuitRequested() 
{
	SaveConfig();
	be_app_messenger.SendMessage(B_QUIT_REQUESTED);
	return(true);
}


status_t ImmersiveVideoWindow::Save(BMessage *message) {
	entry_ref ref;		// For the directory to save into
	status_t err;		// For the return code
	const char *name;	// For the filename
	BPath path;		// For the pathname
	BEntry entry;		// Used to make the path
	FILE *f;		// Standard Posix file


	// If a NULL is passed for the message pointer, use
	// the value we've cached; this lets us do saves without
	// thinking.
	
	if (!message) {
		message = savemessage;
		if (!message) {
			return B_ERROR;
		}
	}
	
	// Peel the entry_ref and name of the directory and
	// file out of the message.
	if ((err=message->FindRef("directory", &ref)) != B_OK) {
		return err;
	}
	if ((err=message->FindString("name", &name)) != B_OK) {
		return err;
	}
	
	// Take the directory and create a pathname out of it
	
	if ((err=entry.SetTo(&ref)) != B_OK) {
		return err;
	}
	entry.GetPath(&path);		// Create a pathname for the directory
	path.Append(name);			// Tack on the filename
	
	// Now we can save the file.
	
	if (!(f = fopen(path.Path(), "w"))) {
		return B_ERROR;
	}
	
	//err = fwrite(textview->Text(), 1, textview->TextLength(), f);
	fclose(f);
	if (err >= 0) {
		SetTitle(name);
		saveitem->SetEnabled(true);
		if (savemessage != message) {
			delete savemessage;
			savemessage = new BMessage(*message);
		}
	}
	return err;
}


void ImmersiveVideoWindow::SetInputBounds(BRect *rect)
{
		/*wird beim laden eine Bildes aufgerufen um die
		 die Größe des Eingabebildes zu setzen*/
		Lock();
		BString tmpString;
		tmpString << (rect->Width()+1);
		inputWidth->SetText(tmpString.String());
		tmpString="";
		tmpString << (rect->Height()+1);
		inputHeight->SetText(tmpString.String());


		if (!unitType)
		{
			tmpString="";
			tmpString << (rect->Width()+1);
			outputWidth->SetText(tmpString.String());
			tmpString="";
			tmpString << (rect->Height()+1);
			outputHeight->SetText(tmpString.String());
		}
		Unlock();
		relation=(rect->Width()/rect->Height());
//		printf("Realtion=\t%f\n",relation);
}

BRect *ImmersiveVideoWindow::GetOutPutBounds() 
{
		if (!unitType)
			return new BRect(0,0,atof(outputWidth->Text())-1,atof(outputHeight->Text())-1);
		else
		{
			float w,h;
			BRect *inB=GetInPutBounds();
			w=(atof(outputWidth->Text())-1)*inB->Width()/100;
			h=(atof(outputHeight->Text())-1)*inB->Height()/100;
			//printf("Bei Prozent:\n\tWidht=%f\n\tHeight=%f\n",w,h);
			return new BRect(0,0,w,h);
		}
}
BRect *ImmersiveVideoWindow::GetInPutBounds() {
	//printf("Bei Pixel:\n\tWidht=%f\n\tHeight=%f\n",atof(inputWidth->Text()),atof(inputHeight->Text()));
	return new BRect(0,0,(atof(inputWidth->Text())-1),(atof(inputHeight->Text())-1));
}

status_t ImmersiveVideoWindow::SaveConfig()
{
	BPath *path=new BPath();
	find_directory(B_COMMON_SETTINGS_DIRECTORY,path,false,NULL);
	path->Append("Immersive Video/");
	return B_OK;
}
