#include "WgAds.h"

WgAds::WgAds(int Ax, int Ay, WgMode Amode) : WgBackground(Ax, Ay, Amode)
{
	system(fmt("touch %s%s", ADS_FILES_PATH, FLAG_FILE_NAME));
	widget_update_time_ = 1000; // 1s
	ads = NULL;
	adsPeace = NULL;
	lineCount = 0;
	fileTime = 0;

	//std::string request="xvfb-run --server-args=\"-screen 0, 1920x1080x24\" cutycapt --url=file:/home/pi/Desktop/Projects/SmartRTU/res/tmp/temp.html --out=/home/pi/Desktop/Projects/SmartRTU/res/tmp/temp.png --min-width=1920 --min-height=1080";

	//std::system(request.c_str());
	//HtmlPic=new Picture("res/tmp/temp.png");
	//printf("width: %i, height: %i\n", HtmlPic->Get_width(), HtmlPic->Get_height());

	std::fprintf(stdout,"%s\tWgAds widget object was created\n", StrNow());
}

WgAds::~WgAds()
{
	

	if (ads){
		delete[] ads;
	}
	if (adsPeace){
		delete[] adsPeace;
	}
	std::fprintf(stdout,"%s\tWgAds widget object was deleted\n", StrNow());
}

time_t WgAds::getFileTime()
{
	//printf("I am in getFileTime\n");
	struct stat buff;
	if (stat(fmt("%s%s", ADS_FILES_PATH, ADS_FILE_NAME), &buff) == 0)
		return buff.st_mtime;
	return 0;
}

bool WgAds::renewFlag()
{
	//printf("I am in renewFlag\n");
	struct dirent **namelist;
	int n = scandir(ADS_FILES_PATH, &namelist, NULL, alphasort);
	if (n < 0)
		perror("scandir");
	else
		while (n--)
		{
			if (strcmp(namelist[n]->d_name, FLAG_FILE_NAME) == 0)
				return true;
		}
	return false;
}

bool WgAds::needRenew()
{
	time_t ft = getFileTime();
	if ((ft != 0 && ft != fileTime )|| renewFlag())
	{
		system(fmt("rm -rf %s%s", ADS_FILES_PATH, FLAG_FILE_NAME));
		fileTime = ft;
		return true;
	}
	return false;
}

bool WgAds::readFile(char *&text)
{
	if (needRenew())
	{
		FILE *file = fopen(fmt("%s%s", ADS_FILES_PATH, ADS_FILE_NAME), "rb");
		fseek(file, 0, SEEK_END);
		int size = ftell(file);

		if (text)
			delete[] text;
		text = new char[size + 1];

		fseek(file, 0, SEEK_SET);
		fread(text, 1, size, file);
		fclose(file);
		text[size] = '\0';

		return true;
	}
	return false;
}

int WgAds::Separate(char *Src, char **Dst)
{
	int p = 0;
	if (Dst)
		Dst[p] = Src;
	for (; *Src; Src++)
	{
		if (*Src == '\n')
		{
			if (Dst)
				*Src = '\0';
			char *str = Src + 1;
			while (*str == '\r')
				*str++;
			if (Dst)
				Dst[++p] = str;
			else
				p++;
		}
	}
	return p;
}

bool WgAds::update()
{
	if (readFile(ads))
	{
		lineCount = Separate(ads) + 1;
		if (adsPeace)
			delete[] adsPeace;
		adsPeace = new char *[lineCount];
		for (int i = 0; i < lineCount; i++)
			adsPeace[i] = NULL;
		Separate(ads, adsPeace);
		std::fprintf(stdout,"%s\tNew %d-lines advertisement text is loaded\n", StrNow(), lineCount);
		return true;
	}
	return false;
}

void WgAds::render()
{
	//TODO
	//SHADOWS MUST BE ON FRONT OF HTML AD NOT ON BACK

	//~~~ render background

	WgBackground::render();

	//~~~ render header

	//RenderWidgetHeader( "Sludinājums" );
	RenderWidgetHeader("Par mums");

	/*
	int width=(float) RectClient.width * 0.9;	
	int height = (float) RectClient.height * 0.9;

	
	//~~~~~ render back shadows
	WgBackground::RenderOnlyShadows();
*/

	//~~~ calculate line count without last empy strings
	/*
	int lc = lineCount;
	for (int i = lc - 1; i >= 0; i--) {
		if (  !adsPeace[i] || adsPeace[i][0] == '\0' ) lc--;
		else break;
	}

	//~~~ render body if exists

	if ( lc == 0 ) return;

	TFont * font = FontStorage->GetFont((char*)"arialBold");
	
	const float lineSpacing = 0.4;
	int width = (float) RectClient.width * 0.9;	
	int height = (float) RectClient.height * 0.9;

	float lh = (float) height / lc;
	float fh = lh * (1. - lineSpacing);
	font->Set_Size( (int)fh );

	int tw = 0;
	for (int i = 0; i < lc; i++) 
	{
		int w = font->TextWidth( adsPeace[i] );
		if ( w > tw ) tw = w;
	}
	if ( tw > width ) {
		lh *= (float)  width / tw;
		fh = lh * (1. - lineSpacing);
	}

	int ofsY = ( RectClient.height - lh * lc - lh * lineSpacing ) / 2 - lh * 0.1;
	SetTextColor(clHaki);
	font->Set_Size( fh )	;
	for (int i = 0; i < lc; i++) 
	{
		font->TextMid(
			adsPeace[i], 
			RectClient.left + RectClient.width / 2, 
			RectClient.top - ofsY - lh * (i + 1)
		);
	}
	*/
}