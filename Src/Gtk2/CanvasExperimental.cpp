#include "../Common/Common.h"

#include <math.h>
#include "Canvas.h"
#include "DasherControl.h"

#include "../DasherCore/DasherTypes.h"

//#undef WITH_CAIRO

//#define IGNAS_NODRAW
//#define IGNAS_RECTANGLE_PRINT
  #define IGNAS_DRAW_MAIN_RECTANGLES true
//  #define IGNAS_NORECTANGLE_DRAW
//  #define IGNAS_NOFONT_DRAW
//#define IGNAS_FRAME_PRINT
//#define IGNAS_NO_BLANK

#ifdef IGNAS_NODRAW
#define IGNAS_STOP_VOID return;
#endif
#ifndef IGNAS_NODRAW
#define IGNAS_STOP_VOID ;
#endif

#ifdef IGNAS_NO_BLANK
#define IGNAS_STOP_BLANK return;
#endif
#ifndef IGNAS_NO_BLANK
#define IGNAS_STOP_BLANK ;
#endif


#ifdef IGNAS_NORECTANGLE_DRAW
#define IGNAS_RECTANGLE_STOP_VOID return;
#endif 
#ifndef IGNAS_NORECTANGLE_DRAW
#define IGNAS_RECTANGLE_STOP_VOID ; 
#endif
#ifdef IGNAS_NOFONT_DRAW
#define IGNAS_FONT_STOP_VOID return;
#endif 
#ifndef IGNAS_NOFONT_DRAW
#define IGNAS_FONT_STOP_VOID ; 
#endif
using namespace Dasher;

CCanvas::CCanvas(GtkWidget *pCanvas, CPangoCache *pPangoCache)
  : CDasherScreen(pCanvas->allocation.width, pCanvas->allocation.height) {

  m_pCanvas = pCanvas;
  m_bLoadBackground = true;
  m_bCaptureBackground = true;
///IGNAS

#ifdef FRAMERATE_DIAGNOSTICS
count_frames=0;
 lasttime=-1;
 count_time=0;
total_frames=0;
 count_rectangles=0;
#endif

	display_depth=3;
	display_fontdepth=4;
	display_fontwidth=m_pCanvas->allocation.width;
	display_fontheight=m_pCanvas->allocation.height;
///IGNAS


#if WITH_CAIRO
  cairo_colours = 0;
#else
  colours = 0;
#endif
  

  m_pPangoCache = pPangoCache;
  
  m_iWidth = m_pCanvas->allocation.width;
  m_iHeight = m_pCanvas->allocation.height;

	display_data= (guchar *) g_malloc(m_pCanvas->allocation.width*m_pCanvas->allocation.height*display_depth * sizeof(guchar));
	display_backgrounddata= (guchar *) g_malloc(m_pCanvas->allocation.width*m_pCanvas->allocation.height*display_depth * sizeof(guchar));

display_fontdata= (guchar *) g_malloc(display_fontwidth*display_fontheight*display_fontdepth * sizeof(guchar));


	point_id= (gint *) g_malloc(m_pCanvas->allocation.width * sizeof(gint));
	point_amount= (gint *) g_malloc(m_pCanvas->allocation.width * sizeof(gint));
	point_data= (gint *) g_malloc(m_pCanvas->allocation.width * sizeof(gint));
GdkColor bg;
GdkColor fg;
bg.red=0;bg.green=0;bg.blue=0; 
fg=bg;
display_pixbuf =gdk_pixbuf_new_from_data  (display_data,
                                             GDK_COLORSPACE_RGB,
                                             false, //no alpha
                                             8,
                                             m_pCanvas->allocation.width,
                                             m_pCanvas->allocation.height,
                                             m_pCanvas->allocation.width*display_depth,
                                             NULL,
                                             NULL);

#if WITH_CAIRO
display_fontcairosurface=cairo_image_surface_create_for_data (display_fontdata,
                                                           CAIRO_FORMAT_RGB24,
                                                           display_fontwidth,display_fontheight,
                                                           display_fontwidth*display_fontdepth);
display_fontcairo= cairo_create(display_fontcairosurface);
#else
//FONT FOR GDK
display_fontgdk= gdk_pixmap_create_from_data (m_pCanvas->window,
                                             (gchar*)display_fontdata,
                                             m_iWidth,
                                             m_iHeight,
                                             24,
                                             &m_pCanvas->style->fg[GTK_STATE_NORMAL],
                                             &m_pCanvas->style->bg[GTK_STATE_NORMAL]);

	gdk_drawable_set_colormap(display_fontgdk, gdk_colormap_get_system());

 if (display_fontgdk==NULL)
	printf("No PIXMAP!!!\n");

#endif

  m_pPangoInk = new PangoRectangle;
  gtk_widget_add_events(m_pCanvas, GDK_ALL_EVENTS_MASK);
}

CCanvas::~CCanvas() {
  // Free the buffer pixmaps


  #if WITH_CAIRO
     cairo_destroy(display_fontcairo);
     
  #else
     g_object_unref(display_fontgdk);
  #endif
  free(display_fontdata);
  free(display_backgrounddata);
  g_object_unref(display_pixbuf);
  free(display_data);
  
  delete m_pPangoInk;
}

void CCanvas::Blank() {
//IGNAS_STOP_VOID

  GdkColor _c;
 
#if WITH_CAIRO
	_c.red=(int)(cairo_colours[0].r*255);		
	_c.green=(int)(cairo_colours[0].g*255);		
	_c.blue=(int)(cairo_colours[0].b*255);
#else
	_c.red=colours[0].red;		
	_c.green=colours[0].green;		
	_c.blue=colours[0].blue;
#endif


  for(int i=0;i<m_iWidth*m_iHeight*display_depth;i+=display_depth)
	{
		display_data[i]=_c.red;
		display_data[i+1]=_c.green;
		display_data[i+2]=_c.blue;
	}

}

void CCanvas::Display() {

//IGNAS_STOP_VOID

  GdkGC *graphics_context;

  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];

  gdk_draw_pixbuf(m_pCanvas->window, graphics_context, display_pixbuf, 0, 0, 0, 0, m_iWidth,m_iHeight, GDK_RGB_DITHER_NORMAL, 0, 0);
}

void CCanvas::DrawRectangle(int x1, int y1, int x2, int y2, int Color, int iOutlineColour, Opts::ColorSchemes ColorScheme, bool bDrawOutline, bool bFill, int iThickness) {
bFill&=IGNAS_DRAW_MAIN_RECTANGLES;
IGNAS_RECTANGLE_STOP_VOID
IGNAS_STOP_VOID

#ifdef FRAMERATE_DIAGNOSTICS
///IGNAS
  count_rectangles++;
///IGNAS
#endif 

  GdkColor _c; 
  int iLeft;
  int iTop;
  int iRight;
  int iBottom;
  int iWidth;
  int iHeight;

  if( x2 > x1 ) {
    iLeft = x1;
    iWidth = x2 - x1;
  }
  else {
    iLeft = x2;
    iWidth = x1 - x2;
  }

  if( y2 > y1 ) {
    iTop = y1;
    iHeight = y2 - y1;
  }
  else {
    iTop = y2;
    iHeight = y1 - y2;
  }
  iBottom=iTop+iHeight;
  if (iBottom> m_iHeight) iBottom=m_iHeight;
  iRight=iLeft+iWidth;
  if (iRight> m_iWidth) iRight=m_iWidth;
  if (iLeft<0) iLeft=0;
  if (iTop<0) iTop=0;
  if(bFill) {
#if WITH_CAIRO
        _c.red=(int)(cairo_colours[Color].r*255);		
	_c.green=(int)(cairo_colours[Color].g*255);		
	_c.blue=(int)(cairo_colours[Color].b*255);
#else
	_c.red=colours[Color].red;		
	_c.green=colours[Color].green;		
	_c.blue=colours[Color].blue;
#endif

    for (int i=iTop;i<iBottom;i++)
    	for(int j=iLeft;j<iRight;j++)
	{
		display_data[i*m_iWidth*display_depth+j*display_depth]=_c.red;
		display_data[i*m_iWidth*display_depth+j*display_depth+1]=_c.green;
		display_data[i*m_iWidth*display_depth+j*display_depth+2]=_c.blue;
	}
  }
  
 if(bDrawOutline) {
    if( iOutlineColour == -1 )
    {


#if WITH_CAIRO
	_c.red=(int)(cairo_colours[3].r*255);		
	_c.green=(int)(cairo_colours[3].g*255);		
	_c.blue=(int)(cairo_colours[3].b*255);
#else
	_c.red=colours[3].red;		
	_c.green=colours[3].green;		
	_c.blue=colours[3].blue;
#endif

    }
    else
{


 #if WITH_CAIRO
	_c.red=(int)(cairo_colours[iOutlineColour].r*255);		
	_c.green=(int)(cairo_colours[iOutlineColour].g*255);		
	_c.blue=(int)(cairo_colours[iOutlineColour].b*255);
#else
	_c.red=colours[iOutlineColour].red;		
	_c.green=colours[iOutlineColour].green;		
	_c.blue=colours[iOutlineColour].blue;
#endif

}

	int i=0,j=0;
	for(int thickness=0;thickness<iThickness;thickness++)
	{
		if (iLeft+thickness<m_iWidth)
		for (i=iTop;i<iBottom;i++)
		{
			
			
			display_data[i*m_iWidth*display_depth+(iLeft+thickness)*display_depth]=_c.red;
			display_data[i*m_iWidth*display_depth+(iLeft+thickness)*display_depth+1]=_c.green;
			display_data[i*m_iWidth*display_depth+(iLeft+thickness)*display_depth+2]=_c.blue;
			
		}
		if ((iRight-thickness>=0)&&(iRight-thickness<m_iWidth))
		for (i=iTop;i<iBottom;i++)
		{
			display_data[i*m_iWidth*display_depth+(iLeft+iWidth-thickness)*display_depth]=_c.red;
			display_data[i*m_iWidth*display_depth+(iLeft+iWidth-thickness)*display_depth+1]=_c.green;
			display_data[i*m_iWidth*display_depth+(iLeft+iWidth-thickness)*display_depth+2]=_c.blue;
			
		}
 		if (iTop+thickness<m_iHeight)
		for(j=iLeft;j<iRight;j++)
                
		{
			display_data[(iTop+thickness)*m_iWidth*display_depth+j*display_depth]=_c.red;
			display_data[(iTop+thickness)*m_iWidth*display_depth+j*display_depth+1]=_c.green;
			display_data[(iTop+thickness)*m_iWidth*display_depth+j*display_depth+2]=_c.blue;
		}
		if ((iBottom-thickness<m_iHeight)&&(iBottom-thickness>=0))
		for(j=iLeft;j<iRight;j++)
 
			display_data[(iTop+iHeight-thickness)*m_iWidth*display_depth+j*display_depth]=_c.red;
			display_data[(iTop+iHeight-thickness)*m_iWidth*display_depth+j*display_depth+1]=_c.green;
			display_data[(iTop+iHeight-thickness)*m_iWidth*display_depth+j*display_depth+2]=_c.blue;
		}	
	}
  

}

void CCanvas::CirclePoints(int cx, int cy, int x, int y, int Colour,int iWidth)
    {
	//A circle drawing algorithm taken from 
	//http://www.cs.unc.edu/~mcmillan/comp136/

        GdkColor _c;
#if WITH_CAIRO
	_c.red=(int)(cairo_colours[Colour].r*255);		
	_c.green=(int)(cairo_colours[Colour].g*255);		
	_c.blue=(int)(cairo_colours[Colour].b*255);
#else
	_c.red=colours[Colour].red;		
	_c.green=colours[Colour].green;		
	_c.blue=colours[Colour].blue;
#endif
        
        if (x == 0) {
            SetPixel(&_c, cx, cy + y,iWidth); //act
            SetPixel(&_c, cx, cy - y,iWidth);
            SetPixel(&_c, cx + y, cy,iWidth);
            SetPixel(&_c, cx - y, cy,iWidth);
	    
        } else 
        if (x == y) {
            SetPixel(&_c, cx + x, cy + y,iWidth);//act
            SetPixel(&_c, cx - x, cy + y,iWidth);
            SetPixel(&_c, cx + x, cy - y,iWidth);
            SetPixel(&_c, cx - x, cy - y,iWidth);
        } else 
        if (x < y) {
            SetPixel(&_c, cx + x, cy + y,iWidth);//act
            SetPixel(&_c, cx - x, cy + y,iWidth);
            SetPixel(&_c, cx + x, cy - y,iWidth);
            SetPixel(&_c, cx - x, cy - y,iWidth);
            SetPixel(&_c, cx + y, cy + x,iWidth);
            SetPixel(&_c, cx - y, cy + x,iWidth);
            SetPixel(&_c, cx + y, cy - x,iWidth);
            SetPixel(&_c, cx - y, cy - x,iWidth);
        }
    }
void CCanvas::HVThinLine(int x0,int y0,int x1,int y1,int Colour)
{
	GdkColor _c;
#if WITH_CAIRO
	_c.red=(int)(cairo_colours[Colour].r*255);		
	_c.green=(int)(cairo_colours[Colour].g*255);		
	_c.blue=(int)(cairo_colours[Colour].b*255);
#else
	_c.red=colours[Colour].red;		
	_c.green=colours[Colour].green;		
	_c.blue=colours[Colour].blue;
#endif
	int start,end,x,y;
	if (x0!=x1)
	{
		if (x0<x1)
		{
			start=x0; 
			end=x1;
		}
		else 
		{
			start = x1;
			end=x0;
		}
                if (start<0) start =0;
		if (start>=m_iWidth) start =m_iWidth-1;

		if ((y0>=0)&&(y0<m_iHeight))
			for(x=start;x<end;x++)
			{
			 	display_data[y0*m_iWidth*display_depth+x*display_depth]=_c.red;
				display_data[y0*m_iWidth*display_depth+x*display_depth+1]=_c.green;
				display_data[y0*m_iWidth*display_depth+x*display_depth+2]=_c.blue;
			}
		
		
	}
	else
	{
		if (y0<y1)
		{
			start=y0; 
			end=y1;
		}
		else 
		{
			start = y1;
			end=y0;
		}
                if (start<0) start =0;
		if (start>=m_iHeight) start =m_iHeight-1;

		if ((x0>=0)&&(x0<m_iWidth))
			for(y=start;y<end;y++)
			{
			 	display_data[y*m_iWidth*display_depth+x0*display_depth]=_c.red;
				display_data[y*m_iWidth*display_depth+x0*display_depth+1]=_c.green;
				display_data[y*m_iWidth*display_depth+x0*display_depth+2]=_c.blue;
			}
		
		
	}
}
void CCanvas::CircleMidpoint(int xCenter, int yCenter, int radius, int Colour,int iWidth)
{
        int x,y,p;
        x = 0;
        y = radius;
        p = (5 - radius*4)/4;
	CirclePoints(xCenter, yCenter, x,y, Colour,iWidth);
        while (x < y) {
            x++;
            if (p < 0) {
                p += 2*x+1;
            } else {
                y--;
                p += 2*(x-y)+1;
            }
	   CirclePoints(xCenter, yCenter, x, y, Colour,iWidth);
	           
        } 
    }
void CCanvas::CircleFill(int xCenter,int yCenter,int radius,int Colour)
{
	GdkColor _c;
#if WITH_CAIRO
	_c.red=(int)(cairo_colours[Colour].r*255);		
	_c.green=(int)(cairo_colours[Colour].g*255);		
	_c.blue=(int)(cairo_colours[Colour].b*255);
#else
	_c.red=colours[Colour].red;		
	_c.green=colours[Colour].green;		
	_c.blue=colours[Colour].blue;
#endif
    int x = 0;
    int y = radius;
    int p = (5 - radius*4)/4;   

    while (x < y) {
       x++;
       if (p < 0) {
                p += 2*x+1;
            } else {
                y--;
                p += 2*(x-y)+1;
            }
	
	    HVThinLine(xCenter-x,yCenter-y,xCenter-x,yCenter+y,Colour);
	    HVThinLine(xCenter+x,yCenter-y,xCenter+x,yCenter+y,Colour);
	    HVThinLine(xCenter-y,yCenter-x,xCenter-y,yCenter+x,Colour);
	    HVThinLine(xCenter+y,yCenter-x,xCenter+y,yCenter+x,Colour);
      }
}
void CCanvas::DrawCircle(screenint iCX, screenint iCY, screenint iR, int iColour, int iFillColour, int iThickness, bool bFill) {


  if (bFill)
	CircleFill(iCX,iCY,iR,iFillColour);
  CircleMidpoint(iCX,iCY,iR,iColour,iThickness);
}
bool CCanvas::HorizontalIntersectionPoint(int h,int a,int b,int x0,int y0,int x1,int y1, int *p)
{
	if (x0==x1) return false;
        if (((h>y0)&&(h>y1))||((h<y0)&&(h<y1))) return false;
	int t;
        if (x0>x1)
	{
		t=x0;
		x0=x1;
		x1=t;
		t=y0;
		y0=y1;
		y1=t;
	}
	int x;
        double l;
	int h1 = y0-h;
        int h2 = y1-h;
	if (h1<0) h1=-h1;
	if (h2<0) h2=-h2;
	int X = x1-x0;
	if (X<0) X = -X;
        if (X==0)
	{	
		
		if ((x0>=a)&&(x0<=b)) *p=x0; 
		else
		{
			if (x0<a) *p=a; else *p=b;
		}
		return true;
	}
        if (h1 == 0)
	{
		if ((x0>=a)&&(x0<=b)) *p=x0; 
		else
		{
			if (x0<a) *p=a; else *p=b;
		}
		return true;
	}
	else
	{
		if (h2==0)
		{
			if ((x1>=a)&&(x1<=b)) *p=x1; 
			else
			{
				if (x1<a) *p=a; else *p=b;
			}
			return true;
		}
		else
		{
			l = (double)(h2 * X)/(double)(h1+h2);
			x = int (x1-l);
			if ((x>=a)&&(x<=b)) *p=x; 
			else
			{
				if (x<a) *p=a; else *p=b;
			}
			return true;
		}
	}
        
}
void CCanvas::PolygonFill(Dasher::CDasherScreen::point *Points, int Number, int Colour)
{
	GdkColor _c;
#if WITH_CAIRO
	_c.red=(int)(cairo_colours[Colour].r*255);		
	_c.green=(int)(cairo_colours[Colour].g*255);		
	_c.blue=(int)(cairo_colours[Colour].b*255);
#else
	_c.red=colours[Colour].red;		
	_c.green=colours[Colour].green;		
	_c.blue=colours[Colour].blue;
#endif
	if (Number>m_iWidth) return; //The size of point_data
	int minx=m_iWidth-1,maxx=0,miny=m_iHeight-1,maxy=0;
        int i;
	for (i=0;i<Number;i++)
	{
		if (Points[i].x<minx) minx = Points[i].x;
		if (Points[i].x>maxx) maxx = Points[i].x;
		if (Points[i].y<miny) miny = Points[i].y;
		if (Points[i].y>maxy) maxy = Points[i].y;
	}
	if (minx<0) minx=0;
	if (miny<0) miny=0;
        if (maxx>=m_iWidth) maxx=m_iWidth-1;
	if (maxy>=m_iHeight) maxy=m_iHeight-1;
	int p;
        int depth;
        for (i=0;i<m_iWidth;i++)
		point_id[i]=-1;
	for(int scanline=miny;scanline<=maxy;scanline++)
	{
		int found=-1;	
		for(i=1;i<Number;i++)
		{
			if (HorizontalIntersectionPoint(scanline,0,m_iWidth-1,Points[i-1].x,Points[i-1].y,Points[i].x,Points[i].y,&p))
			{
				found++;
				point_data[found]=p;	
			}
		}
		if (HorizontalIntersectionPoint(scanline,0,m_iWidth-1,Points[Number-1].x,Points[Number-1].y,Points[0].x,Points[0].y,&p))
			{
				found++;
				point_data[found]=p;	
			}
                for (i=0;i<found;i++)
		{
			if (point_id[point_data[i]]!=scanline) 
			{	
				point_amount[point_data[i]]=1;
				point_id[point_data[i]]=scanline;
			}
                            else point_amount[point_data[i]]++;
		}
		depth=0;
		for (i=0;i<m_iWidth;i++)
		{
			if (point_id[i]!=scanline)
			{
				if (depth%2==1) 
				{
					display_data[scanline*m_iWidth*display_depth+i*display_depth]=_c.red;
					display_data[scanline*m_iWidth*display_depth+i*display_depth+1]=_c.green;
					display_data[scanline*m_iWidth*display_depth+i*display_depth]=_c.blue;
				}
			}
			else
			{
				depth+=point_amount[i];
			}
		}
	}
}
void CCanvas::Polygon(Dasher::CDasherScreen::point *Points, int Number, int Colour, int iWidth) {
IGNAS_STOP_VOID
  
  for(int i = 1; i < Number; i++) 
  {
      Line(Points[i].x,Points[i].y,Points[i-1].x,Points[i-1].y, iWidth,Colour);
  }
  Line(Points[Number-1].x,Points[Number-1].y,Points[0].x,Points[0].y,iWidth,Colour);
}
void CCanvas::SetPixel(GdkColor *c,int x,int y,int iWidth)
{

   int x0=x,y0=y;
   for (int i=0;i<iWidth;i++)
   for (int j=0;j<iWidth;j++)
   {

	   x=x0+i-iWidth/2;
	   y=y0+j-iWidth/2;
	   if (((y>=0)&&(y<m_iHeight))&&((x>=0)&&(x<m_iWidth)))
	   {
		
		   display_data[y*m_iWidth*display_depth+x*display_depth]=c->red;
		   display_data[y*m_iWidth*display_depth+x*display_depth+1]=c->green;
		   display_data[y*m_iWidth*display_depth+x*display_depth+2]=c->blue;
	   }
	   
	   
   }

}


void CCanvas::Line(int x0,int y0,int x1,int y1,int iWidth, int Colour)
{
//A Bresenham line drawing algorithm taken from 
//http://www.cs.unc.edu/~mcmillan/comp136/Lecture6/Lines.html

	GdkColor _c;
#if WITH_CAIRO
	_c.red=(int)(cairo_colours[Colour].r*255);		
	_c.green=(int)(cairo_colours[Colour].g*255);		
	_c.blue=(int)(cairo_colours[Colour].b*255);
#else
	_c.red=colours[Colour].red;		
	_c.green=colours[Colour].green;		
	_c.blue=colours[Colour].blue;
#endif
        int dy = y1 - y0;
        int dx = x1 - x0;
        int stepx, stepy;

        if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
        if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
        dy <<= 1;                                                  // dy is now 2*dy
        dx <<= 1;                                                  // dx is now 2*dx

        SetPixel(&_c, x0, y0,iWidth);
        if (dx > dy) {
            int fraction = dy - (dx >> 1);                         // same as 2*dy - dx
            while (x0 != x1) {
                if (fraction >= 0) {
                    y0 += stepy;
                    fraction -= dx;                                // same as fraction -= 2*dx
                }
                x0 += stepx;
                fraction += dy;                                    // same as fraction -= 2*dy
                SetPixel(&_c, x0, y0,iWidth);
            }
        } else {
            int fraction = dx - (dy >> 1);
            while (y0 != y1) {
                if (fraction >= 0) {
                    x0 += stepx;
                    fraction -= dy;
                }
                y0 += stepy;
                fraction += dx;
                SetPixel(&_c, x0, y0,iWidth);
            }
        }
    
  
}
void CCanvas::Polyline(Dasher::CDasherScreen::point *Points, int Number, int iWidth, int Colour) {

IGNAS_STOP_VOID



  for(int i = 1; i < Number; i++) 
  {
      Line(Points[i].x,Points[i].y,Points[i-1].x,Points[i-1].y, iWidth,Colour);
  }
}

void CCanvas::DrawString(const std::string &String, int x1, int y1, int size) {
IGNAS_FONT_STOP_VOID
IGNAS_STOP_VOID


#if WITH_CAIRO  //set color black
    my_cairo_colour_t _c = cairo_colours[4];		
    cairo_set_source_rgb(display_fontcairo, _c.r, _c.g, _c.b);

#else 
	GdkGC *graphics_context;
       graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];
#endif //WITH_CAIRO

#if WITH_CAIRO
  PangoLayout *pLayout(m_pPangoCache->GetLayout(display_fontcairo, String, size));
#else
  
  PangoLayout *pLayout(m_pPangoCache->GetLayout(GTK_WIDGET(m_pCanvas), String, size));

#endif



  pango_layout_get_pixel_extents(pLayout, m_pPangoInk, NULL);

   int height=m_pPangoInk->height/2; 
 guchar back=255;
 int value,black;
 double alpha;
  for (int i=m_pPangoInk->y;i<m_pPangoInk->y+m_pPangoInk->height;i++)
    if ((i+y1-height<m_iHeight) &&(i+y1-height>=0))
	for(int j=m_pPangoInk->x;j<+m_pPangoInk->x+m_pPangoInk->width;j++)
	if ((j+x1>=0)&&(j+x1<m_iWidth))
	{
		display_fontdata[i*display_fontwidth*display_fontdepth+j*display_fontdepth]=back;
		display_fontdata[i*display_fontwidth*display_fontdepth+j*display_fontdepth+1]=back;
		display_fontdata[i*display_fontwidth*display_fontdepth+j*display_fontdepth+2]=back;
	}

#if WITH_CAIRO
  pango_cairo_show_layout(display_fontcairo, pLayout);
#else

  GdkColormap* colormap=gdk_colormap_get_system();
  GdkColor _c = colours[4];				
    gdk_colormap_alloc_color(colormap, &_c, FALSE, TRUE);	
    gdk_gc_set_foreground (graphics_context, &_c);
  gdk_draw_layout(display_fontgdk, graphics_context, 0,0, pLayout);

#endif


  for (int i=m_pPangoInk->y;i<m_pPangoInk->y+m_pPangoInk->height;i++)
 	if ((i+y1-height<m_iHeight) &&(i+y1-height>=0))
  {
	for(int j=m_pPangoInk->x;j<+m_pPangoInk->x+m_pPangoInk->width;j++)
	if ((j+x1>=0)&&(j+x1<m_iWidth))
	{
		if (display_fontdata[i*display_fontwidth*display_fontdepth+j*display_fontdepth+0]!=back)		
{
	value = display_fontdata[i*display_fontwidth*display_fontdepth+j*display_fontdepth+0];
        alpha =(double)value/256.0;
        black = (int )(value * (1-alpha));
       
display_data[(i+y1-height)*m_iWidth*display_depth+(j+x1)*display_depth]=int (display_data[(i+y1-height)*m_iWidth*display_depth+(j+x1)*display_depth]*alpha)+black;

display_data[(i+y1-height)*m_iWidth*display_depth+(j+x1)*display_depth+1]=int (display_data[(i+y1-height)*m_iWidth*display_depth+(j+x1)*display_depth+1]*alpha)+black;

display_data[(i+y1-height)*m_iWidth*display_depth+(j+x1)*display_depth+2]=int (display_data[(i+y1-height)*m_iWidth*display_depth+(j+x1)*display_depth+2]*alpha)+black;

}
	}

  } 
}

void CCanvas::TextSize(const std::string &String, int *Width, int *Height, int size) {

#if WITH_CAIRO
  PangoLayout *pLayout(m_pPangoCache->GetLayout(display_fontcairo, String, size));
#else
  PangoLayout *pLayout(m_pPangoCache->GetLayout(GTK_WIDGET(m_pCanvas), String, size));
#endif
  pango_layout_get_pixel_extents(pLayout, m_pPangoInk, NULL);

  *Width = m_pPangoInk->width;
  *Height = m_pPangoInk->height;

}

void CCanvas::SendMarker(int iMarker) {
  switch(iMarker) {
  case 0: // Switch to display buffer
    break;
  case 1: // Switch to decorations buffer
    StoreBackground();
    LoadBackground();	
#ifdef FRAMERATE_DIAGNOSTICS
    NewFrame();
#endif
    break;
  }
}

void CCanvas::SetColourScheme(const CColourIO::ColourInfo *pColourScheme) {
  int iNumColours(pColourScheme->Reds.size());

#if WITH_CAIRO
  if (cairo_colours)
    delete[] cairo_colours;
  cairo_colours = new my_cairo_colour_t[iNumColours];
#else
  if (colours)
    delete[] colours;
  colours = new GdkColor[iNumColours];
#endif

  for(int i = 0; i < iNumColours; i++) {
#if WITH_CAIRO
    cairo_colours[i].r = pColourScheme->Reds[i] / 255.0;
    cairo_colours[i].g = pColourScheme->Greens[i] / 255.0;
    cairo_colours[i].b = pColourScheme->Blues[i] / 255.0;
#else
    colours[i].pixel=0;
    colours[i].red=pColourScheme->Reds[i]*257;
    colours[i].green=pColourScheme->Greens[i]*257;
    colours[i].blue=pColourScheme->Blues[i]*257;
#endif
  }
}
void CCanvas::SetLoadBackground(bool value)
{
	m_bLoadBackground=value;
}
void CCanvas::SetCaptureBackground(bool value)
{
	m_bCaptureBackground=value;
}
void CCanvas::StoreBackground()
{
  if (m_bCaptureBackground)
    {
      for (int i=0;i<m_iWidth*m_iHeight*display_depth;i++)
	display_backgrounddata[i]=display_data[i];	
      m_bCaptureBackground=false;
    }
}

void CCanvas::LoadBackground()
{
	if (m_bLoadBackground)
	{
		for (int i=0;i<m_iWidth*m_iHeight*display_depth;i++)
			display_data[i]=display_backgrounddata[i];	
	}

}

bool CCanvas::GetCanvasSize(GdkRectangle *pRectangle)
{
  if ((pRectangle == NULL) || (m_pCanvas == NULL))
    return false;
 
  // Using gtk_window_get_frame_extents() only seems to return the position
  // and size of the parent Dasher window.  So we'll get the widgets position
  // and use its size to determine the bounding rectangle.
  int iX = 0;
  int iY = 0;

  gdk_window_get_position(m_pCanvas->window, &iX, &iY);

  pRectangle->x       = iX;
  pRectangle->y       = iY;
  pRectangle->width   = m_iWidth;
  pRectangle->height  = m_iHeight;

  return true;
}
void CCanvas::ChangeState(int value)
{
	
}

#ifdef FRAMERATE_DIAGNOSTICS
void CCanvas::NewFrame()
{
	count_frames++;
	time_t seconds=time(NULL);
	float average=0;
	if (lasttime==-1) lasttime=seconds;
	time_t temp=seconds-lasttime;
	
	if (temp>=1)
	{
		count_time += temp;
		if (count_time!=1)
		{
			total_frames+=count_frames;
			average=(float)total_frames/(float)(count_time-1);
		}
		printf("New second %ld had %d frames, the averages is %f, total frames=%ld\n ",count_time,count_frames,average,total_frames);
		count_frames=0;
	}
#ifdef IGNAS_FRAME_PRINT	
	printf("OLD FRAME%d\n",count_rectangles);
#endif
	count_rectangles=0;
	lasttime=seconds;

}

#endif
