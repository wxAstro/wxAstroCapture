#ifndef WXPROPGRID_H
#define WXPROPGRID_H

#include <wx/grid.h>
#include <vector>
#include <list>


/*
    This is a derived wxGrid with some specific characteristics different from wxGrid
    - There are only two columns
    - Column labels are as soecified, not A, B
    - Row labels are hidden
    - First column is a read only keword, second column is an editable value
*/

class wxPropGrid : public wxGrid
{
   public:
      wxPropGrid(wxWindow *parent,
                 wxWindowID id,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxWANTS_CHARS,
                 const wxString& name = wxGridNameStr);

      virtual ~wxPropGrid();

      typedef std::vector<wxString> Columns;
      struct Entry {
         Entry() {}
         Entry(const wxString& kw, const wxString& vl, const wxString& co)
         : keyword(kw),value(vl),comment(co) {}
         wxString keyword;
         wxString value;
         wxString comment;
      };
      typedef std::list<Entry> Entries;
      bool CreateGrid(const Entries& entries,  // initial grid entries
                      const Columns& columns,  // column headers (columns.size() must be 2)
                      wxGrid::wxGridSelectionModes selmode = wxGrid::wxGridSelectCells);

      bool CreateChoice(const wxString& kw,const wxArrayString& choices);

      // retrieve the values from the grid
      void GetCellValues(Entries& entries);

   protected:
   private:
   //   Columns m_columns;
};



#endif // WXPROPGRID_H
