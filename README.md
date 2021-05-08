# log
Very basic logging functionality

This library prints messages in the following format:
`DEBUG 00:00:00 | @ function:42 | some message`

Call `ta_init_log(filename, enable_color)` before logging anything.<br>
The filename parameter specifies a file to write the logged message to, if you pass NULL to this parameter, file logging is disabled.<br>
The enable_color parameters specifies whether the log levels (INFO, WARNING, ERROR, etc.) should be colored.<br><br>

Call `ta_free_log()` at the end of your application, right now this only closes the log file if you're using one<br><br>

Logging is done using the following macros:
* `RML_DBG`: Debug
* `RML_INF`: Info
* `RML_WRN`: Warn
* `RML_ERR`: Error
* `RML_CRT`: Critical/Fatal

These macros can be used just like printf, with a format strings followed by optional arguments

The `RML_DBG` macro is disabled in release mode, it can still be enabled by defining `RML_DEBUG_OVERRIDE`
