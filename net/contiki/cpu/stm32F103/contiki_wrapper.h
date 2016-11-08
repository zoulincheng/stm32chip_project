
//#ifdef CONTIKIF_WRAPPER

#ifndef CONTIKIF_WRP_INCLUDE
#define CONTIKIF_WRP_INCLUDE 
// prefix all function calls and definitions by CONTIKI_

// rename FILE to CONTIKIFILE 
#define FILE CONTIKI_FILE

// take care of _P calls
#define _write_P(...) _write(__VA_ARGS__)

// 
#define CONTIKI_STDIO_PREFIXED

// prefix all stdio calls with CONTIKI_
#define clearerr(...) CONTIKI_clearerr(__VA_ARGS__)
#define fclose(...) CONTIKI_fclose(__VA_ARGS__)
#define feof(...) CONTIKI_feof(__VA_ARGS__)
#define ferror(...) CONTIKI_ferror(__VA_ARGS__)
#define fflush(...) CONTIKI_fflush(__VA_ARGS__)
#define fgetc(...) CONTIKI_fgetc(__VA_ARGS__)
#define fgets(...) CONTIKI_fgets(__VA_ARGS__)
#define fopen(...) CONTIKI_fopen(__VA_ARGS__)
#define fprintf(...) CONTIKI_fprintf(__VA_ARGS__)
#define fputc(...) CONTIKI_fputc(__VA_ARGS__)
#define fputs(...) CONTIKI_fputs(__VA_ARGS__)
#define fpurge(...) CONTIKI_fpurge(__VA_ARGS__)
#define fread(...) CONTIKI_fread(__VA_ARGS__)
#define freopen(...) CONTIKI_freopen(__VA_ARGS__)
#define fscanf(...) CONTIKI_fscanf(__VA_ARGS__)
#define fseek(...) CONTIKI_fseek(__VA_ARGS__)
#define ftell(...) CONTIKI_ftell(__VA_ARGS__)
#define fwrite(...) CONTIKI_fwrite(__VA_ARGS__)
#define getc(...) CONTIKI_getc(__VA_ARGS__)

#ifdef getchar
#undef getchar
#endif

#define getchar(...) CONTIKI_getchar(__VA_ARGS__)

#define gets(...) CONTIKI_gets(__VA_ARGS__)
#define printf(...) CONTIKI_printf(__VA_ARGS__)
#define putc(...) CONTIKI_putc(__VA_ARGS__)

#ifdef putchar
#undef putchar
#endif

#define putchar(...) CONTIKI_putchar(__VA_ARGS__)
#define puts(...) CONTIKI_puts(__VA_ARGS__)
#define scanf(...) CONTIKI_scanf(__VA_ARGS__)
#define sprintf(...) CONTIKI_sprintf(__VA_ARGS__)
#define sscanf(...) CONTIKI_sscanf(__VA_ARGS__)
#define ungetc(...) CONTIKI_ungetc(__VA_ARGS__)
#define vfprintf(...) CONTIKI_vfprintf(__VA_ARGS__)
#define vfscanf(...) CONTIKI_vfscanf(__VA_ARGS__)
#define vsprintf(...) CONTIKI_vsprintf(__VA_ARGS__)
#define vsscanf(...) CONTIKI_vsscanf(__VA_ARGS__)
#define fprintf_P(...) CONTIKI_fprintf(__VA_ARGS__)
#define fputs_P(...) CONTIKI_fputs(__VA_ARGS__)
#define fscanf_P(...) CONTIKI_fscanf(__VA_ARGS__)
#define fwrite_P(...) CONTIKI_fwrite(__VA_ARGS__)
#define printf_P(...) CONTIKI_printf(__VA_ARGS__)
#define puts_P(...) CONTIKI_puts(__VA_ARGS__)
#define scanf_P(...) CONTIKI_scanf(__VA_ARGS__)
#define sprintf_P(...) CONTIKI_sprintf(__VA_ARGS__)
#define sscanf_P(...) CONTIKI_sscanf(__VA_ARGS__)
#define vfprintf_P(...) CONTIKI_vfprintf(__VA_ARGS__)
#define vfscanf_P(...) CONTIKI_vfscanf(__VA_ARGS__)
#define vsprintf_P(...) CONTIKI_vsprintf(__VA_ARGS__)
#define vsscanf_P(...) CONTIKI_vsscanf(__VA_ARGS__)

#define fileno(...) CONTIKI__fileno(__VA_ARGS__)
#define fcloseall(...) CONTIKI_fcloseall(__VA_ARGS__)

#define _fileno(...) CONTIKI__fileno(__VA_ARGS__)
#define _fdopen(...) CONTIKI__fdopen(__VA_ARGS__)
#define _flushall(...) CONTIKI__flushall(__VA_ARGS__)
#define _fmode(...) CONTIKI__fmode(__VA_ARGS__)

#define errno verrno


#if 0
#ifdef stdin
#undef stdin
#define stdin (__iob[0])        /*!< \brief Standard input stream. */
#endif

#ifdef stdout
#undef stdout
#define stdout (__iob[0])       /*!< \brief Standard output stream. */
#endif

#ifdef stderr
#undef stderr
#define stderr (__iob[0])       /*!< \brief Standard error output stream. */
#endif
#endif

#endif//CONTIKIF_WRP_INCLUDE

//#endif//CONTIKIF_WRAPPER

