///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2012, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------
//
//	exrdisplay -- a simple program to display Imf::Rgba multipart
//                    and deep images
//                 -- exrdisplay Window Mouse Control:
//                      LMB = Display a sample chart and print out values
//                      RMB = If it's a deep image, open a Deep 3D Window
//                 -- Deep 3D Window Mouse Control:
//                      LMB = Rotate
//                      RMB = Zoom
//                      MMB = Move
//                 -- Deep 3D Window Control Keys:
//                      a = scale z value up
//                      s = scale z value down
//                      f = reset to fit
//                      d = decrease pixel samples
//                      c = increase pixel samples
//
//-----------------------------------------------------------------------------

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Output.H>

#include <ImfArray.h>
#include <ImfThreading.h>
#include <ImfMultiPartInputFile.h>
#include <ImathLimits.h>

#include <iostream>
#include <algorithm>
#include <string>
#include <exception>
#include <string.h>
#include <stdlib.h>

#include "loadImage.h"
#include "scaleImage.h"
#include "applyCtl.h"
#include "ImageView.h"
#include "namespaceAlias.h"

using namespace IMF;
using namespace IMATH;
using namespace std;


struct MainWindow
{
    Fl_Window *         window;
    Fl_Choice *         multipartMenu;
    Fl_Output *         typeLabel;
    Fl_Output *         nameLabel;
    Fl_Box *            exposureLabel;
    Fl_Value_Slider *   exposureSlider;
    Fl_Box *            defogLabel;
    Fl_Value_Slider *   defogSlider;
    Fl_Box *            kneeLowLabel;
    Fl_Value_Slider *   kneeLowSlider;
    Fl_Box *            kneeHighLabel;
    Fl_Value_Slider *   kneeHighSlider;
    Fl_Box *            rgbaBox;
    ImageView *         image;
    Array<Rgba>         pixels;
    Array<float*>       dataZ;
    Array<unsigned int> sampleCount;
    const char*         imageFile;
    bool                preview;
    int                 lx;
    int                 ly;
    const char*         channel;
    const char*         layer;
    bool                swap;
    float               farPlane;
    bool                deepComp;

    static void         multipartComboboxCallback (Fl_Widget *widget, void *data);
    static void         exposureSliderCallback (Fl_Widget *widget, void *data);
    static void         defogSliderCallback (Fl_Widget *widget, void *data);
    static void         kneeLowSliderCallback (Fl_Widget *widget, void *data);
    static void         kneeHighSliderCallback (Fl_Widget *widget, void *data);
};

void
MainWindow::multipartComboboxCallback (Fl_Widget *widget, void *data)
{
    MainWindow *mainWindow = (MainWindow *) data;
    int partnum = mainWindow->multipartMenu->value();

    mainWindow->image->clearDataDisplay();

    //
    // reload pixels
    //
    Header header;
    int zsize;

    loadImage (mainWindow->imageFile,
               mainWindow->channel,
	       mainWindow->layer,
	       mainWindow->preview,
	       mainWindow->lx,
	       mainWindow->ly,
	       partnum,
               zsize,
	       header,
	       mainWindow->pixels,
	       mainWindow->dataZ,
	       mainWindow->sampleCount,
	       mainWindow->deepComp);

    const Box2i &displayWindow = header.displayWindow();
    const Box2i &dataWindow = header.dataWindow();

    int dw = dataWindow.max.x - dataWindow.min.x + 1;
    int dh = dataWindow.max.y - dataWindow.min.y + 1;
    int dx = dataWindow.min.x - displayWindow.min.x;
    int dy = dataWindow.min.y - displayWindow.min.y;

    if (mainWindow->swap)
	swapPixels (dw, dh, mainWindow->pixels);

    if(mainWindow->preview)
    {
        int w = mainWindow->window->w();
        int iw = displayWindow.max.x - displayWindow.min.x;
        int ih = displayWindow.max.y - displayWindow.min.y;

        mainWindow->window->size (w, (160 + ih));
        mainWindow->image->resize ((w - iw) / 2, 155, iw, ih);
    }
    mainWindow->image->setPixels (mainWindow->pixels,
                                  mainWindow->dataZ,
                                  mainWindow->sampleCount,
                                  zsize,
                                  dw,dh,dx,dy);

    //
    // renew multipart data type
    //
    string type = "";
    try{
    	type = header.type();
    }
    catch(IEX_NAMESPACE::BaseExc &e) {
        type = "";
    }
    mainWindow->typeLabel->value(type.c_str());

    //
    // renew multipart part name
    //
    string name = "";
    try{
        name = header.name();
    }
    catch(IEX_NAMESPACE::BaseExc &e) {
        name = "";
    }
    mainWindow->nameLabel->value(name.c_str());
}


void	
MainWindow::exposureSliderCallback (Fl_Widget *widget, void *data)
{
    MainWindow *mainWindow = (MainWindow *) data;
    mainWindow->image->setExposure (mainWindow->exposureSlider->value());
}


void	
MainWindow::defogSliderCallback (Fl_Widget *widget, void *data)
{
    MainWindow *mainWindow = (MainWindow *) data;
    mainWindow->image->setDefog (mainWindow->defogSlider->value());
}


void	
MainWindow::kneeLowSliderCallback (Fl_Widget *widget, void *data)
{
    MainWindow *mainWindow = (MainWindow *) data;
    mainWindow->image->setKneeLow (mainWindow->kneeLowSlider->value());
}


void	
MainWindow::kneeHighSliderCallback (Fl_Widget *widget, void *data)
{
    MainWindow *mainWindow = (MainWindow *) data;
    mainWindow->image->setKneeHigh (mainWindow->kneeHighSlider->value());
}


MainWindow *
makeMainWindow (const char imageFile[],
                const char channel[],
                const char layer[],
                bool preview,
                int lx,
                int ly,
                float farPlane,
                bool noDisplayWindow,
                bool noAspect,
                bool zeroOneExposure,
                bool normalize,
                bool swap,
                bool continuousUpdate,
                const vector<string> &transformNames,
                bool useCtl,
                bool deepComp)
{
    MainWindow *mainWindow = new MainWindow;
    mainWindow->imageFile = imageFile;
    mainWindow->preview = preview;
    mainWindow->lx = lx;
    mainWindow->ly = ly;
    mainWindow->farPlane = farPlane;
    mainWindow->channel = channel;
    mainWindow->layer = layer;
    mainWindow->swap = swap;
    mainWindow->deepComp = deepComp;

    //
    // Read the image file.
    //

    int numparts = 0;

    try
    {
    	MultiPartInputFile *infile = new MultiPartInputFile(imageFile);
        numparts = infile->parts();
        delete infile;
    }
    catch(IEX_NAMESPACE::BaseExc &e)
    {
        cerr<<"\n"<<"ERROR:"<<endl;
        cerr<<e.what()<<endl;
        exit(1);
    }

    Header header;
    int zsize;

    //
    //pass 0 as partnum for the first load
    //
    loadImage (imageFile,
               channel,
               layer,
               preview,
               lx, ly,
               0,
               zsize,
               header,
               mainWindow->pixels,
               mainWindow->dataZ,
               mainWindow->sampleCount,
               mainWindow->deepComp);

    const Box2i &displayWindow = header.displayWindow();
    const Box2i &dataWindow = header.dataWindow();
    float pixelAspectRatio = header.pixelAspectRatio();

    int w  = displayWindow.max.x - displayWindow.min.x + 1;
    int h  = displayWindow.max.y - displayWindow.min.y + 1;
    int dw = dataWindow.max.x - dataWindow.min.x + 1;
    int dh = dataWindow.max.y - dataWindow.min.y + 1;
    int dx = dataWindow.min.x - displayWindow.min.x;
    int dy = dataWindow.min.y - displayWindow.min.y;

    if (noDisplayWindow)
    {
        w = dw;
        h = dh;
        dx = 0;
        dy = 0;
    }

    if (noAspect)
    {
	pixelAspectRatio = 1;
    }

    //
    // Normalize the pixel data if necessary.
    //

    if (normalize)
	normalizePixels (dw, dh, mainWindow->pixels);

    //
    // If necessary, swap the top and bottom half and then the
    // left and right half of the image.
    // 

    if (swap)
	swapPixels (dw, dh, mainWindow->pixels);

    //
    // Stretch the image horizontally or vertically to make the
    // pixels square (assuming that we are going to display the
    // image on a screen with square pixels).
    //

    if (pixelAspectRatio > 1)
	scaleX (pixelAspectRatio, w, h, dw, dh, dx, dy, mainWindow->pixels);
    else
	scaleY (1 / pixelAspectRatio, w, h, dw, dh, dx, dy, mainWindow->pixels);

    //
    // Apply CTL transforms if requested.
    //
    // If we don't apply CTL transforms and we have loaded more than
    // one image channel, then transform the pixels from the RGB space
    // of the input file to into the RGB space of the display.
    //

#if HAVE_CTL_INTERPRETER

    if (useCtl)
    {
	applyCtl (transformNames,
		  header,
		  mainWindow->pixels,
		  dw, dh,
		  mainWindow->pixels);
    }
    else

#endif

    if (!channel)
    {
        adjustChromaticities (header,
                              mainWindow->pixels,
                              dw, dh,
                              mainWindow->pixels);
    }

    //
    // Build main window
    //

    int mw = max (500, w);	// main window width
    int vy = 0;			// offset of image view from top of main window

    float exposure = 0;
    float defog = 0;
    float kneeLow = 0;
    float kneeHigh = 0;

#if HAVE_CTL_INTERPRETER

    if (useCtl)
    {
	//
	// Colors on the screeen are computed by CTL.
	// No exposure defog and knee sliders are displayed.
	//

	mainWindow->window =
	    new Fl_Window (mw + 10, h + 35, imageFile);

	//
	// Add RGB value display
	//

	mainWindow->rgbaBox = new Fl_Box (80, 5, mw - 65, 20, "");
	mainWindow->rgbaBox->align (FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

	//
	// Image view is below RGB value display
	//

	vy = 30;

	//
	// Map floating-point pixel values 0.0 and 1.0
	// to the display's white and black respectively.
	//

	exposure = 1.02607;
	defog = 0;
	kneeLow = 0.0;
	kneeHigh = 3.5;
    }
    else

#endif

    {
        mainWindow->window =
                        new Fl_Window (mw + 10, h + 160, imageFile);

        //
        // Add multipart combobox
        //

        mainWindow->multipartMenu =
                        new Fl_Choice (70, 5, 80, 20, "multipart");

        for(int i=0; i<numparts; i++){
            string displaynum;
            ostringstream s;
            s << i;
            mainWindow->multipartMenu->add(s.str().c_str());
        }

        mainWindow->multipartMenu->value(0);

        mainWindow->multipartMenu->callback
        (MainWindow::multipartComboboxCallback, mainWindow);

        if(numparts==1)
            mainWindow->multipartMenu->deactivate();
        else{

            //
            // Add type label
            //

            string type = "";

            try{
                type = header.type();
            }
            catch(IEX_NAMESPACE::BaseExc &e) {
                type = "";
            }

            mainWindow->typeLabel =
                            new Fl_Output (190, 5, 110, 20, "type");
            mainWindow->typeLabel->value(type.c_str());

            //
            // Add name label
            //

            string name = "";

            try{
                name = header.name();
            }
            catch(IEX_NAMESPACE::BaseExc &e) {
                name = "";
            }

            mainWindow->nameLabel =
                            new Fl_Output (350, 5, 155, 20, "name");
            mainWindow->nameLabel->value(name.c_str());
        }

        //
        // Add exposure slider
        //

        mainWindow->exposureLabel =
                        new Fl_Box (5, 30, 60, 20, "exposure");

        mainWindow->exposureSlider =
                        new Fl_Value_Slider (70, 30, mw - 65, 20, "");

        enum Fl_When when = continuousUpdate?
                        FL_WHEN_CHANGED : FL_WHEN_RELEASE;

        mainWindow->exposureSlider->type (FL_HORIZONTAL);
        mainWindow->exposureSlider->range (-10.0, +10.0);
        mainWindow->exposureSlider->step (1, 8);
        exposure = zeroOneExposure? 1.02607: 0.0;
        mainWindow->exposureSlider->value (exposure);
        mainWindow->exposureSlider->when (when);

        mainWindow->exposureSlider->callback
        (MainWindow::exposureSliderCallback, mainWindow);

        //
        // Add defog slider
        //

        mainWindow->defogLabel =
                        new Fl_Box (5, 55, 60, 20, "defog");

        mainWindow->defogSlider =
                        new Fl_Value_Slider (70, 55, mw - 65, 20, "");

        mainWindow->defogSlider->type (FL_HORIZONTAL);
        mainWindow->defogSlider->range (0.0, 0.01);
        mainWindow->defogSlider->step (1, 10000);
        defog = 0.0;
        mainWindow->defogSlider->value (defog);
        mainWindow->defogSlider->when (when);

        mainWindow->defogSlider->callback
        (MainWindow::defogSliderCallback, mainWindow);

        //
        // Add kneeLow slider
        //

        mainWindow->kneeLowLabel =
                        new Fl_Box (5, 80, 60, 20, "knee low");

        mainWindow->kneeLowSlider =
                        new Fl_Value_Slider (70, 80, mw - 65, 20, "");

        mainWindow->kneeLowSlider->type (FL_HORIZONTAL);
        mainWindow->kneeLowSlider->range (-3.0, 3.0);
        mainWindow->kneeLowSlider->step (1, 8);
        kneeLow = 0.0;
        mainWindow->kneeLowSlider->value (kneeLow);
        mainWindow->kneeLowSlider->when (when);

        mainWindow->kneeLowSlider->callback
        (MainWindow::kneeLowSliderCallback, mainWindow);

        //
        // Add kneeHigh slider
        //

        mainWindow->kneeHighLabel =
                        new Fl_Box (5, 105, 60, 20, "knee high");

        mainWindow->kneeHighSlider =
                        new Fl_Value_Slider (70, 105, mw - 65, 20, "");

        mainWindow->kneeHighSlider->type (FL_HORIZONTAL);
        mainWindow->kneeHighSlider->range (3.5, 7.5);
        mainWindow->kneeHighSlider->step (1, 8);
        kneeHigh = (preview | zeroOneExposure)? 3.5: 5.0;
        mainWindow->kneeHighSlider->value (kneeHigh);
        mainWindow->kneeHighSlider->when (when);

        mainWindow->kneeHighSlider->callback
        (MainWindow::kneeHighSliderCallback, mainWindow);

        //
        // Add RGB value display
        //

        mainWindow->rgbaBox = new Fl_Box (80, 130, mw - 65, 20, "");
        mainWindow->rgbaBox->align (FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

        //
        // Image view is below RGB value display
        //

        vy = 155;
    }

    //
    // Add image view:
    //
    // w, h		width and height of the display window
    //
    // dw, dh		width and height of the data window
    //
    // dx, dy		offset of the data window's upper left
    // 			corner from the display window's upper
    // 			left corner
    //

    mainWindow->image =
                    new ImageView (5 + (mw - w) / 2, vy,
                                   w, h,
                                   "",
                                   mainWindow->pixels,
                                   mainWindow->dataZ,
                                   mainWindow->sampleCount,
                                   zsize,
                                   dw, dh,
                                   dx, dy,
                                   mainWindow->rgbaBox,
                                   mainWindow->farPlane,
                                   displayVideoGamma(),
                                   exposure,
                                   defog,
                                   kneeLow,
                                   kneeHigh);

    mainWindow->image->box (FL_ENGRAVED_BOX);

    mainWindow->window->end();

    return mainWindow;
}


void
usageMessage (const char argv0[], bool verbose = false)
{
    cerr << "usage: " << argv0 << " [options] imagefile" << endl;

    if (verbose)
    {
        cerr << "\n"
        "Displays an OpenEXR image on the screen.\n"
        "\n"
        "Options:\n"
        "\n"
        "-p        displays the preview (thumbnail)\n"
        "          image instead of the main image\n"
        "\n"
        "-L x      displays layer x of a multilayer image\n"
        "\n"
        "-l lx ly  displays level (lx,ly) of a tiled\n"
        "          multiresolution image\n"
        "\n"
        "-w        displays all pixels in the data window,\n"
        "          ignoring the display window\n"
        "\n"
        "-a        ignores the image's pixel aspect ratio,\n"
        "          and does not scale the image to make\n"
        "          the pixels square\n"
        "\n"
        "-c x      loads only image channel x\n"
        "\n"
        "-1        sets exposure and knee sliders so that pixel\n"
        "          value 0.0 becomes black, and 1.0 becomes white\n"
        "\n"
        "-n        normalizes the pixels so that the smallest\n"
        "          value becomes 0.0 and the largest value\n"
        "          becomes 1.0\n"
        "\n"
        "-A        same as -c A -1 (displays alpha)\n"
        "\n"
        "-Z        same as -c Z -n (displays depth)\n"
        "\n"
        "-s        swaps the image's top and bottom half, then\n"
        "          swaps the left and right half, so that the\n"
        "          four corners of the image end up in the center.\n"
        "          (Useful for checking the seams of wrap-around\n"
        "          texture map images.)\n"
#if HAVE_CTL_INTERPRETER
        "\n"
        "-C s      CTL transform s is applied to the image before\n"
        "          it is displayed.  Option -C can be specified\n"
        "          multiple times to apply a series of transforms\n"
        "          to the image.  The transforms are applied in the\n"
        "          order in which they appear on the command line.\n"
        "\n"
        "-T        do not apply CTL transforms to the image; enable\n"
        "          interactive exposure and knee controls instead\n"
        "\n"
        "-u        changing the exposure and knee controls\n"
        "          continuously updates the on-screen image\n"
        "          (the controls are enabled only when no CTL\n"
        "          transforms have been applied to the image)\n"
        "\n"
        "-t n      use n parallel threads to read the image file\n"
        "          and to run the CTL transforms\n"
#else
        "\n"
        "-u        changing the exposure and knee controls\n"
        "          continuously updates the on-screen image\n"
        "\n"
        "-t n      use n parallel threads to read the image file\n"
#endif
        "\n"
        "-h        prints this message\n"
#if HAVE_CTL_INTERPRETER
        "\n"
        "CTL transforms:\n"
        "\n"
        "       CTL transforms are applied to the image unless\n"
        "       one of the following options is specified on the\n"
        "       command line: -c, -1, -n, -A, -Z, -T\n"
        "\n"
        "       If one or more CTL transforms are specified on\n"
        "       the command line (using the -C flag), then those\n"
        "       transforms are applied to the image.\n"
        "       If no CTL transforms are specified on the command\n"
        "       line then an optional look modification transform\n"
        "       is applied, followed by a rendering transform and\n"
        "       a display transform.\n"
        "       The name of the look modication transform is taken\n"
        "       from the lookModTransform attribute in the header\n"
        "       of the image file.  If the header contains no such\n"
        "       attribute, then no look modication transform is\n"
        "       applied.  The name of the rendering transform is\n"
        "       taken from the renderingTransform attribute in the\n"
        "       header of the image file.  If the header contains\n"
        "       no such attribute, then the name of the rendering\n"
        "       transform is \"transform_RRT.\"  The name of the\n"
        "       display transform is taken from the environment\n"
        "       variable CTL_DISPLAY_TRANSFORM.  If this environment\n"
        "       variable is not set, then the name of the display\n"
        "       transform is \"transform_display_video.\"\n"
        "       The files that contain the CTL code for the\n"
        "       transforms are located using the CTL_MODULE_PATH\n"
        "       environment variable.\n"
#endif
        "\n"
        "Deep Data Options:\n"
        "\n"
        "-farPlane(f) f    OpenGL zFar clipping plane\n"
        "-noDeepComp(d)    Disable the compositing of deep images (def=ON)\n"
        "\n"
        "Exrdisplay Window Mouse Control:\n"
        "                  LMB = Display a sample chart and print out values\n"
        "                  RMB = If it's a deep image, open a Deep 3D Window\n"
        "Deep 3D Window Mouse Control:\n"
        "                  LMB = Rotate\n"
        "                  RMB = Zoom\n"
        "                  MMB = Move\n"
        "Deep 3D Window Control Keys:\n"
        "                  a = scale z value up\n"
        "                  s = scale z value down\n"
        "                  f = reset to fit\n"
        "                  d = decrease pixel samples\n"
        "                  c = increase pixel samples\n"
        "\n";

        cerr << endl;
    }

    exit (1);
}

void window_callback(Fl_Widget*, void*)
{
    exit(0);
}

int
main(int argc, char **argv)
{
    const char *imageFile = 0;
    const char *channel = 0;
    const char *layer = 0;
    bool preview = false;
    bool noDisplayWindow = false;
    bool noAspect = false;
    bool zeroOneExposure = false;
    bool normalize = false;
    bool swap = false;
    bool continuousUpdate = false;
    vector<string> transformNames;
    bool useCtl = true;
    int numThreads = 0;
    
    int lx = -1;
    int ly = -1;
    float farPlane = limits<float>::max(); //default value of zfar plane
    bool deepComp = true;  // attempt to composite deep images together

    //
    // Parse the command line.
    //

    if (argc < 2)
        usageMessage (argv[0], true);

    int i = 1;

    while (i < argc)
    {
        if (!strcmp (argv[i], "-p"))
        {
            //
            // Show the preview image
            //

            preview = true;
            i += 1;
        }
        else if (!strcmp (argv[i], "-L"))
        {
            //
            // Assume that the image file has multiple
            // layers, and show the specified layer.
            //

            if (i > argc - 2)
                usageMessage (argv[0]);

            layer = argv[i + 1];
            i += 2;
        }
        else if (!strcmp (argv[i], "-l"))
        {
            //
            // Assume that the image file is tiled,
            // and show level (lx,ly) of the tiled image
            //

            if (i > argc - 3)
                usageMessage (argv[0]);

            lx = strtol (argv[i + 1], 0, 0);
            ly = strtol (argv[i + 2], 0, 0);
            i += 3;
        }
        else if (!strcmp (argv[i], "-w"))
        {
            //
            // Ignore the file's display window
            //

            noDisplayWindow = true;
            i += 1;
        }
        else if (!strcmp (argv[i], "-a"))
        {
            //
            // Ignore the file's pixel aspect ratio
            //

            noAspect = true;
            i += 1;
        }
        else if (!strcmp (argv[i], "-c"))
        {
            //
            // Load only one image channel.
            //

            if (i > argc - 2)
                usageMessage (argv[0]);

            channel = argv[i + 1];
            useCtl = false;
            i += 2;
        }
        else if (!strcmp (argv[i], "-1"))
        {
            //
            // Display 0.0 to 1.0 range.
            //

            zeroOneExposure = true;
            useCtl = false;
            i += 1;
        }
        else if (!strcmp (argv[i], "-n"))
        {
            //
            // Normalize pixels.
            //

            zeroOneExposure = true;
            normalize = true;
            useCtl = false;
            i += 1;
        }
        else if (!strcmp (argv[i], "-A"))
        {
            //
            // Display alpha
            //

            zeroOneExposure = true;
            normalize = false;
            channel = "A";
            useCtl = false;
            i += 1;
        }
        else if (!strcmp (argv[i], "-Z"))
        {
            //
            // Display depth
            //

            zeroOneExposure = true;
            normalize = true;
            channel = "Z";
            useCtl = false;
            i += 1;
        }
        else if (!strcmp (argv[i], "-s"))
        {
            //
            // Swap top and bottom half, then left and right half.
            //

            swap = true;
            i += 1;
        }
        else if (!strcmp (argv[i], "-u"))
        {
            //
            // Continuous update.
            //

            continuousUpdate = true;
            i += 1;
        }
        else if (!strcmp (argv[i], "-C"))
        {
            //
            // Apply a CTL transform
            //

            if (i > argc - 2)
                usageMessage (argv[0]);

            transformNames.push_back (argv[i + 1]);
            i += 2;
        }
        else if (!strcmp (argv[i], "-T"))
        {
            //
            // No CTL transforms.
            //

            useCtl = false;
            i += 1;
        }
        else if (!strcmp (argv[i], "-t"))
        {
            //
            // Set number of threads
            //

            if (i > argc - 2)
                usageMessage (argv[0]);

            numThreads = strtol (argv[i + 1], 0, 0);

            if (numThreads < 0)
            {
                cerr << "Number of threads cannot be negative." << endl;
                return 1;
            }

            i += 2;
        }
        else if (!strcmp (argv[i], "-h"))
        {
            //
            // Print help message
            //

            usageMessage (argv[0], true);
        }
        else if (!strcmp (argv[i], "-farPlane") || !strcmp (argv[i], "-f"))
        {
            //
            // zFar plane for display deep data
            //

            if (i > argc - 2)
                usageMessage (argv[0]);

            farPlane = strtod (argv[i + 1], 0);
            i += 2;

            if (farPlane <= 0)
            {
                cerr << "Value of far Plane cannot be negative." << endl;
                exit (1);
            }

        }
        else if (!strcmp (argv[i], "-noDeepComp") || !strcmp (argv[i], "-d"))
        {
            //
            // Toogle the option to composite deep images, else just use
            // the first sample of each pixel
            //
            deepComp = false;
            i += 1;

        }
        else
        {
            //
            // image file name
            //

            imageFile = argv[i];
            i += 1;
        }
    }

    if (imageFile == 0)
        usageMessage (argv[0]);

    //
    // Load the specified image file,
    // open a window on the screen, and
    // display the image.
    //

    int exitStatus = 0;

    try
    {
        setGlobalThreadCount (numThreads);

        MainWindow *mainWindow = makeMainWindow (imageFile,
                                                 channel,
                                                 layer,
                                                 preview,
                                                 lx, ly,
                                                 farPlane,
                                                 noDisplayWindow,
                                                 noAspect,
                                                 zeroOneExposure,
                                                 normalize,
                                                 swap,
                                                 continuousUpdate,
                                                 transformNames,
                                                 useCtl,
                                                 deepComp);

        mainWindow->window->show (1, argv);

        Fl::set_color (FL_GRAY,  240, 240, 240);
        Fl::set_color (FL_GRAY0, 80, 80, 80);

        mainWindow->window->callback(window_callback); // set main window exit

        exitStatus = Fl::run();
    }
    catch (const exception &e)
    {
        cerr << e.what() << endl;
        exitStatus = 1;
    }

    return exitStatus;
}
