(1)Moving to 2.5branch from 2.1branch.
   - Some of header files are changed, so be sure to replace with the latest.
   - Header files are under alien-include folder in the PC version library.

(2)Now FVDK library supports password locked office documents(docx, pptx, xlsx)
   - When an password protected file is passed in, FileViewer library asks the password of the doc
     by calling AlienUserRequest_request(), and then the host app responses with PicselUserRequest_notify().

     * Please see requestPasswordCallback() in gtk-alien-callbacks.c as an example implementation.
