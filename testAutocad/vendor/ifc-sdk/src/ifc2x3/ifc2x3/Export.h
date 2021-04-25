
#ifndef IFC2X3_EXPORT_H
#define IFC2X3_EXPORT_H

#ifdef IFC2X3_STATIC
#  define IFC2X3_EXPORT
#  define IFC2X3_NO_EXPORT
#else
#  ifndef IFC2X3_EXPORT
#    ifdef ifc2x3_EXPORTS
        /* We are building this library */
#      define IFC2X3_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define IFC2X3_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef IFC2X3_NO_EXPORT
#    define IFC2X3_NO_EXPORT 
#  endif
#endif

#ifndef IFC2X3_DEPRECATED
#  define IFC2X3_DEPRECATED __declspec(deprecated)
#endif

#ifndef IFC2X3_DEPRECATED_EXPORT
#  define IFC2X3_DEPRECATED_EXPORT IFC2X3_EXPORT IFC2X3_DEPRECATED
#endif

#ifndef IFC2X3_DEPRECATED_NO_EXPORT
#  define IFC2X3_DEPRECATED_NO_EXPORT IFC2X3_NO_EXPORT IFC2X3_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef IFC2X3_NO_DEPRECATED
#    define IFC2X3_NO_DEPRECATED
#  endif
#endif

#endif /* IFC2X3_EXPORT_H */
