#include <stdio.h>
#include "ImmersiveVideoPlugin.h"
#include "PluginItem.h"


/*PluginItem ein Eintrag in der Liste Plugins ... 
speichert auch die Aktuellen Plugins*/
PluginItem::PluginItem(ImmersiveVideoPlugin* pPlugin) : BListItem()
{
  	plugin=pPlugin;
}

//wird aufgerufen, wenn ein Listeneintrag gezeichnet werden soll
void PluginItem::DrawItem(BView *owner, BRect frame, bool complete)
{
	/*setzen des Zeichenmodus, dieser liefert bessere Ergebnisse für 
	Schrift die mit Antialias gezeichnet wird
	*/
	owner->SetDrawingMode(B_OP_OVER);
	owner->Window()->Lock();
	if (IsSelected() || complete) 
	{
		/*wenn selektiert dann als erstes eine blaues Feld malen und dann
		ein schwarzes Viereck darum farbe kHighlight ist im Header definiert*/
		if (IsSelected())
		{
			owner->SetHighColor(kHighlight);
			owner->FillRect(frame);
			owner->SetHighColor(kBlackColor);
			owner->StrokeRect(frame);

		}
		else {
			/*wenn nicht selektiert ein Feld mit der Hintergrundfarbe des Views
			 malen, damit wird eine eventuelle Blaues Feld 
			 wenn es gerade selektiert war gelöscht
			 nicht ganz effektiv aber hier braruche wir keine Optimierung*/
			owner->SetHighColor(owner->ViewColor());
			owner->FillRect(frame);
		}
//		owner->SetLowColor(color);

	}
	/*falls es disabel wird die Farbe zum zeichnen der Schrift 
	auf Grau anstatt Schwarz  setzen, als visuelles Feedback für
	deb Nutzer*/
	if (IsEnabled())
	{
		owner->SetHighColor(kBlackColor);
	}
	else
	{
		owner->SetHighColor(kMedGray);
	}
//	BFont* font=new BFont();
//	owner->GetFont(font);
	font_height fh;
//	font->GetHeight(&fh);
	owner->GetFontHeight(&fh);
	//anhad der Fontgröße den Text in die Mitte der Zeile platzieren 
	float rest=frame.Height()-fh.ascent;
	owner->MovePenTo(frame.left+25, frame.bottom-rest/2);
	//und den Namen Zeichnen
	owner->DrawString(plugin->GetName());
	/*testen ob ein gültiges Icon von dem Plugin zurückgeliefert wird.*/
	if (plugin->GetIcon()!=NULL) 
	{
		//printf("Icon zeichnen\n");
		bitmapPoint.Set(frame.left+4,frame.top+1);	
/*		BRect source(0,0,15.0,15.0);
		BRect dest(0,0,10.0,10.0);
		dest.OffsetTo(bitmapPoint);*/
		owner->DrawBitmap(plugin->GetIcon(),bitmapPoint);
	}
	owner->Window()->Unlock();
}

ImmersiveVideoPlugin* PluginItem::GetPlugin()
{
	//liefert das in dem Item gespeicherte Plugin zurück
	return plugin;
}

void PluginItem::Update(BView *owner, const BFont *font)
{
	/*Workaround um die Größe des Listeneintrages auf die Größe 
	des Icon an zu passen. Normalerweise wird in Update die Größe
	des Listeneintrages auf die Größe der verwendeten Schrift gesetzt.*/
	BListItem::Update(owner,font);
	SetHeight(17);
}
