
#ifndef STEP_EXPORT_H
#define STEP_EXPORT_H

#ifdef IFC2X3_STATIC
#  define STEP_EXPORT
#  define STEP_NO_EXPORT
#else
#  ifndef STEP_EXPORT
#    ifdef Step_EXPORTS
        /* We are building this library */
#      define STEP_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define STEP_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef STEP_NO_EXPORT
#    define STEP_NO_EXPORT 
#  endif
#endif

#ifndef STEP_DEPRECATED
#  define STEP_DEPRECATED __declspec(deprecated)
#endif

#ifndef STEP_DEPRECATED_EXPORT
#  define STEP_DEPRECATED_EXPORT STEP_EXPORT STEP_DEPRECATED
#endif

#ifndef STEP_DEPRECATED_NO_EXPORT
#  define STEP_DEPRECATED_NO_EXPORT STEP_NO_EXPORT STEP_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef STEP_NO_DEPRECATED
#    define STEP_NO_DEPRECATED
#  endif
#endif

#endif /* STEP_EXPORT_H */
