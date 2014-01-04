#include "wxPropGrid.h"



static wxColour read_only_bg(0,0,0);
static wxColour edit_bg(0,0,100);
//static wxColour choice_bg(55,55,0);
static wxColour choice_bg(150,0,150);

static wxColour read_only_txt(255,255,255);
static wxColour edit_txt(255,255,0);

wxPropGrid::wxPropGrid(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size , long style, const wxString& name )
: wxGrid(parent,id,pos,size,style,name)
{
   //ctor
}

wxPropGrid::~wxPropGrid()
{
   //dtor
}


bool wxPropGrid::CreateGrid(const Entries& entries,  // initial grid entries
                const Columns& columns,  // column headers (columns.size() must be at least 2)
                wxGrid::wxGridSelectionModes selmode)
{
   if(wxGrid::CreateGrid(entries.size(),columns.size(),selmode )) {

      for(int icol=0; icol<int(columns.size()); icol++) {
         SetColLabelValue(icol,columns[icol]);
      }


      SetRowLabelSize(0);
      int irow = 0;
      Entries::const_iterator itrow = entries.begin();
      while(itrow != entries.end() ) {
         SetCellValue(irow, 0,itrow->keyword );  SetReadOnly(irow, 0 );
         SetCellValue(irow, 1,itrow->value );
         SetCellValue(irow, 2,itrow->comment );  // SetReadOnly(irow, 2 );

         SetCellBackgroundColour(irow,0,read_only_bg);
         SetCellTextColour(irow,0,read_only_txt);
         SetCellBackgroundColour(irow,1,edit_bg);
         SetCellTextColour(irow,1,edit_txt);
         SetCellBackgroundColour(irow,2,edit_bg);
         SetCellTextColour(irow,2,edit_txt);
         itrow++; irow++;
      }

      AutoSizeColumns();
      SetColSize(1, 190 );

      return true;
   }
   return false;

}


void wxPropGrid::GetCellValues(Entries& entries)
{
   int nrow = GetNumberRows();
   for(int irow=0; irow<nrow; irow++) {
      entries.push_back(wxPropGrid::Entry(GetCellValue(irow,0),GetCellValue(irow,1),GetCellValue(irow,2)));
   }
}

bool wxPropGrid::CreateChoice(const wxString& kw,const wxArrayString& choices)
{

   int nrow = GetNumberRows();
   for(int irow=0; irow<nrow; irow++) {
      wxString value = GetCellValue(irow,0);
      if(kw == value) {
         SetCellBackgroundColour(irow,1,choice_bg);
         SetCellEditor(irow,1,new wxGridCellChoiceEditor(choices));
         Refresh();
         return true;
      }
   }
   return false;
}
