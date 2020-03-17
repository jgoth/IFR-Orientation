// Downloaded from https://developer.x-plane.com/code-sample/create-instructions-widget/


// Instructions Widget

/*
	PullDown Menu Creates Widget Window to Disply Plugin Instructions or Notes.
		
	This example shows how to use a pulldown menu to create a widget window to display a list of instructions or notes.  
	The widget window is distroyed by clicking a close box.   There are several other widget window features that
	can be explored;  please see the SDK documentation.  
	
	This content added by Blue Side Up Bob.  
*/

// #define XPLM200 = 1;  This example does not require SDK2.0.

#include <httplib.h>
#include "picojson.h"
//#include <boost/beast/http.hpp>
#include "XPLMUtilities.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMGraphics.h"
#include "XPLMNavigation.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#include <http.h>
//using namespace httplib;
int gMenuItem;

char InstructionsText[50][200] = {  
"   1. Text line one.",   
"   2. Text.",
"   3. Text.",  
"   4. Text.",  
"   5. Text.", 
"   6. Text.",  
"end"

};


XPWidgetID	InstructionsWidget = NULL; 
XPWidgetID	InstructionsWindow = NULL;
XPWidgetID	InstructionsTextWidget[50] = {NULL};

void InstructionsMenuHandler(void *, void *);
void CreateInstructionsWidget(int x1, int y1, int w, int h);
int InstructionsHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t  inParam1, intptr_t  inParam2);


void get_waypoint_info(std::string icao, std::string approach_name, std::string transit_point, double distance, double bearing, double& latitude, double& longitude, double& altitude, double& spawn_latitude, double& spawn_longitude) {
	httplib::Client cli("192.168.86.28", 5555);
	httplib::Params params;
	params.emplace("airport_icao", icao);
	params.emplace("approach_name", approach_name);
	params.emplace("transit_point", transit_point);
	params.emplace("distance", std::to_string(distance));
	params.emplace("bearing", std::to_string(bearing));
	auto res = cli.Post("/waypoint/retrieve", params);
	//auto res = cli.Get("/waypoint/retrieve");
	if (res) {
		if (res->status == 200) {
			std::string resbody = res->body;
			const char* json = resbody.c_str();
			std::string err;
			std::cout << json << std::endl;
			picojson::value v;
			const char* json_end = picojson::parse(v, json, json + strlen(json), &err);
			if (!err.empty()) {
				std::cerr << err << std::endl;
			}
			else {
				picojson::object& o = v.get<picojson::object>();
				picojson::object& o2 = o["coordinates"].get<picojson::object>();
				picojson::object& o3 = o2["spawn"].get<picojson::object>();

				latitude = o2["lat"].get<double>();
				longitude = o2["long"].get<double>();
				altitude = o2["altitude"].get<double>();
				spawn_latitude = o3["lat"].get<double>();
				spawn_longitude = o3["long"].get<double>();
				//for (picojson::object::const_iterator i = o2.begin(); i != o2.end(); ++i) {
				//    std::cout << i->first << "  " << i->second << std::endl;
				//}
			}
		}
		else {
			std::cout << "failed" << res->status << std::endl;
		}
	}
	else {
		std::cout << "res is NULL!" << std::endl;
	}

}



PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
	XPLMMenuID	g_menu_id;
	int			g_menu_container_idx;

	strcpy(outName, "Instructions");
	strcpy(outSig, "BlueSideUpBob.Example.Instructions");
	strcpy(outDesc, "A plugin to display an Instruction Text window from the pull down menu.");

// Create our menu
	XPLMDebugString("Creating menu...\n");

	g_menu_container_idx = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "Instructions Plugin", 0, 0);
	g_menu_id = XPLMCreateMenu("Instructions Plugin", XPLMFindPluginsMenu(), g_menu_container_idx, InstructionsMenuHandler, NULL);
	XPLMAppendMenuItem(g_menu_id, "Teleport Me", (void *)"Menu Item 1", 1);
	XPLMAppendMenuSeparator(g_menu_id);
	XPLMAppendMenuItem(g_menu_id, "Instructions", (void*)"Menu Item 2", 1);
	XPLMDebugString("Finished creating menu...\n");

// Flag to tell us if the widget is being displayed.
	gMenuItem = 0;
	
	char buf[80];
	XPLMNavRef ref;
	//ref = XPLMFindNavAid("IKTOW", NULL, NULL, NULL, NULL, xplm_Nav_Fix);
	ref = XPLMFindNavAid("RDU", NULL, NULL, NULL, NULL, xplm_Nav_VOR);
	float outLong, outLat, outHeight, outHeading;
	int outFrequency;
	char outID[25], outReg[500], name[255];
	XPLMNavType outType;
	XPLMGetNavAidInfo(ref, &outType, &outLat, &outLong, &outHeight, &outFrequency, &outHeading, outID, name, outReg/*NULL*/);
	XPLMDebugString("Got navaid info\n");
	//sprintf(buf, "%f", outHeight);
	int ret;
	ret = snprintf(buf, sizeof buf, "%f", outHeight);
	XPLMDebugString(buf);
	XPLMDebugString("\n");
	ret = snprintf(buf, sizeof buf, "%d", outFrequency);
	XPLMDebugString(buf);
	XPLMDebugString("\n");
	//const char* r_override = "sim/operation/override/override_planepath";
	//strcpy(r_override, );

	int tmp[1] = { 1 };
	XPLMSetDatavi("sim/operation/override/override_planepath", &(tmp[0]), 0, 1);

	return 1;
}

PLUGIN_API void	XPluginStop(void)
{

	if (gMenuItem == 1)
	{
		XPDestroyWidget(InstructionsWidget, 1);
		gMenuItem = 0;
	}
}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, long inMsg, void * inParam)
{
}


void InstructionsMenuHandler(void * inMenuRef, void * inItemRef)
{

	//double lat = 35.2372659;	// Latitude
	//double lon = -120.6425974;	// Longitude
	//double alt = 5000 / 3.28084;	// Altitude MSL
	std::string icao("KTTA");
	std::string approach_name("R21");
	std::string transit_point("OZOPE");
	double distance = 50.0;
	double bearing = 260.0;
	double latitude, longitude, altitude, spawn_latitude, spawn_longitude;
	get_waypoint_info(icao, approach_name, transit_point, distance, bearing, latitude, longitude, altitude, spawn_latitude, spawn_longitude);
	// LHR centre
	double lat = spawn_latitude;
	double lon = spawn_longitude;
	//double lat = 51.4706001;	// Latitude
	//double lon = -0.461941;	// Longitude
	//double alt = 0.0;	// On da ground
	double alt = altitude;
	std::string msg = "Spawning to: " + std::to_string(lat) + " " + std::to_string(lon) + " " + std::to_string(alt) + "\n";
	//double alt = 10000.0;	// Way up in air
	XPLMDebugString(msg.c_str());
	XPLMDataRef gPlanePosEUSx = XPLMFindDataRef("sim/flightmodel/position/local_x");
	XPLMDataRef gPlanePosEUSy = XPLMFindDataRef("sim/flightmodel/position/local_y");
	XPLMDataRef gPlanePosEUSz = XPLMFindDataRef("sim/flightmodel/position/local_z");
	float phi = 0;	// No bank
	float psi = 0;	// Heading

	//double time = XPLMGetDataf("sim/time/total_running_time_secs");
	//double theta = 10.0 * sin(time);
	float theta = 0;
	double local_x, local_y, local_z;
	XPLMWorldToLocal(lat, lon, alt, &local_x, &local_y, &local_z);
	
	XPLMDebugString("Checking strings...\n");

	if (!strcmp((const char*)inItemRef, "Menu Item 1")) {
		XPLMDebugString("Teleporting...\n");
		//XPLMSetDatad("sim/flightmodel/position/local_x", local_x);
		//XPLMSetDatad("sim/flightmodel/position/local_y", local_y);
		//XPLMSetDatad("sim/flightmodel/position/local_z", local_z);
		XPLMSetDatad(gPlanePosEUSx, local_x);
		XPLMSetDatad(gPlanePosEUSy, local_y);
		XPLMSetDatad(gPlanePosEUSz, local_z);

		//XPLMSetDataf("sim/flightmodel/position/phi", phi);
		//XPLMSetDataf("sim/flightmodel/position/psi", psi);
		//XPLMSetDataf("sim/flightmodel/position/theta", theta);
		//XPLMSetDataf("sim/flightmodel/position/psi", 215.0);
		XPLMDebugString("Teleporting complete\n");
	}
	/*
	else {
		switch ((int)inItemRef)
		{

		case 1:	if (gMenuItem == 0)
		{
			CreateInstructionsWidget(50, 712, 974, 662);	//left, top, right, bottom.
			gMenuItem = 1;
		}
			  else
		{
			if (!XPIsWidgetVisible(InstructionsWidget))
				XPShowWidget(InstructionsWidget);
		}
			  break;
		}
	}*/
}						


// This will create our widget dialog.
void CreateInstructionsWidget(int x, int y, int w, int h)
{
int Index;

	int x2 = x + w;
	int y2 = y - h;
	
// Create the Main Widget window.
	InstructionsWidget = XPCreateWidget(x, y, x2, y2,
					1,										// Visible
					"INSTRUCTIONS     Blue Side Up Bob",	// desc
					1,										// root
					NULL,									// no container
					xpWidgetClass_MainWindow);


// Add Close Box to the Main Widget.  Other options are available.  See the SDK Documentation.  
	XPSetWidgetProperty(InstructionsWidget, xpProperty_MainWindowHasCloseBoxes, 1);


// Print each line of instructions.
	for (Index=0; Index < 50; Index++)
	{
	if(strcmp(InstructionsText[Index],"end") == 0) {break;}

		// Create a text widget
		InstructionsTextWidget[Index] = XPCreateWidget(x+10, y-(30+(Index*20)) , x2-10, y-(42+(Index*20)),
		1,	// Visible
		InstructionsText[Index],// desc
		0,		// root
		InstructionsWidget,
		xpWidgetClass_Caption);
	}
		
													
							
// Register our widget handler
	XPAddWidgetCallback(InstructionsWidget, InstructionsHandler);
}





// This is our widget handler.  In this example we are only interested when the close box is pressed.
int  InstructionsHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t  inParam1, intptr_t  inParam2)
{
	if (inMessage == xpMessage_CloseButtonPushed)
	{
		if (gMenuItem == 1)
		{
			XPHideWidget(InstructionsWidget);
		}
		return 1;
	}

	return 0;
}

