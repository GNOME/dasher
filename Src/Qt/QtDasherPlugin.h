#include <qutfcodec.h>
#include <qframe.h>
#include "QtDasherScreen.h"
#include "DasherInterface.h"
#include "DashEdit.h"

class QtDasherPlugin : public QFrame, public CDashEditbox
{
  Q_OBJECT
public:
  QtDasherPlugin(QWidget* parent=0, const char* name=0, WFlags f=0);
  ~QtDasherPlugin();

  void resetState();
  QSize sizeHint() const;

  void write_to_file() {};
  void get_new_context(std::string&, int) {};
  void unflush();
  void output(int);
  void deletetext();
  void flush(int);
  void Clear() {};
  void SetEncoding(Dasher::Opts::FileEncodingFormats) {};
  void SetFont(std::string Name, long Size) {};

 signals:
  void key( ushort, ushort, ushort, bool, bool);

 private:
  QtDasherScreen *d;
  CDasherInterface *interface;
  int flushcount;
  QUtf8Codec *utf8_codec;
};







