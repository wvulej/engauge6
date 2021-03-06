#ifndef LOGGER_UPLOAD_H
#define LOGGER_UPLOAD_H

#include <QtGlobal>

class MainWindow;

/// Upload logging information to website for developer support
class LoggerUpload
{
public:
  /// Single constructor
  LoggerUpload();

  /// Bind to MainWindow so this class can access the command stack
  static void bindToMainWindow (const MainWindow *mainWindow);

  /// Smart equivalent to standard assert method and Q_ASSERT (in qglobal.h). Upon error, an upload is proposed.
  /// This is static for easy access from anywhere else in the application
  static void loggerAssert(const char *condition,
                           const char* file,
                           int line) Q_NORETURN;

  /// Smart equivalent to Q_CHECK_PTR (in qglobal.h). Similar to loggerAssert but for checking newly-allocated pointers
  static void loggerCheckPtr (const char *pointer,
                              const char* file,
                              int line) Q_NORETURN;

private:

  static void loggerOutput (const char* comment,
                            const char *file,
                            int line,
                            const char* context) Q_NORETURN;

  static const MainWindow *m_mainWindow;
};

#endif // LOGGER_UPLOAD_H
