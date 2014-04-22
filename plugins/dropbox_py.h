#ifndef __PYX_HAVE__dropbox_py
#define __PYX_HAVE__dropbox_py


#ifndef __PYX_HAVE_API__dropbox_py

#ifndef __PYX_EXTERN_C
  #ifdef __cplusplus
    #define __PYX_EXTERN_C extern "C"
  #else
    #define __PYX_EXTERN_C extern
  #endif
#endif

__PYX_EXTERN_C DL_IMPORT(void) py_main(void);
__PYX_EXTERN_C DL_IMPORT(void) update(void);
__PYX_EXTERN_C DL_IMPORT(PyObject) *longpoll(int (*)(char *, int));
__PYX_EXTERN_C DL_IMPORT(char) *py_open(char *);
__PYX_EXTERN_C DL_IMPORT(int) py_write(char *, FILE *);
__PYX_EXTERN_C DL_IMPORT(int) py_cpy(char *, char *);

#endif /* !__PYX_HAVE_API__dropbox_py */

#if PY_MAJOR_VERSION < 3
PyMODINIT_FUNC initdropbox_py(void);
#else
PyMODINIT_FUNC PyInit_dropbox_py(void);
#endif

#endif /* !__PYX_HAVE__dropbox_py */
