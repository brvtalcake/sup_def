#define _GNU_SOURCE 1

#define _FORTIFY_SOURCE 2

#define _LARGEFILE64_SOURCE 1
#define _LARGEFILE_SOURCE 1
#define _FILE_OFFSET_BITS 64

#define _TIME_BITS 64

#include <sys/types.h>
#include <unistd.h>

#include <stddef.h>
#include <stdckdint.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <threads.h>
#include <stdatomic.h>
#include <pthread.h>
#include <semaphore.h>
#include <regex.h>
#include <signal.h>
#include <argp.h>
#include <getopt.h>

#include <curl/curl.h>
#include <brotli/encode.h>

#undef noreturn
#undef thread_local
#undef nullptr
#undef nullptr_t
#undef constexpr

#undef unlikely
#undef likely
#undef ARRAY_SIZE
#undef ATOMIC_ACCESS
#undef MANUAL_ATOMIC_ACCESS
#undef DEFAULT_UNICODE_VERSION
#undef DEFAULT_OUTPUT_DIR
#undef MAX_CODEPOINT
#undef CODEPOINT_COUNT

// Those are valid keywords in C23 but editors may not colorize them appropriately,
// so define them as macros so they're AT LEAST colored as such.
#define noreturn noreturn
#define thread_local thread_local
#define nullptr nullptr
#define nullptr_t nullptr_t
#define constexpr constexpr

#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x)   __builtin_expect(!!(x), 1)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define ATOMIC_ACCESS _Atomic
#define MANUAL_ATOMIC_ACCESS /* Just to make it clear that we shall use atomic_load() etc to access the value */
#define DEFAULT_UNICODE_VERSION "15.1.0"
#define DEFAULT_OUTPUT_DIR      "generated"
#define MAX_CODEPOINT           0x10FFFF
#define CODEPOINT_COUNT         (MAX_CODEPOINT + 1)

enum general_category
#if __STDC_VERSION__ > 201710L
    : uint64_t
#endif
{
#undef  flags_bits
#define flags_bits(n) (UINT64_C(1) << (n))

    GENERAL_CATEGORY_Lu = flags_bits(0),
    GENERAL_CATEGORY_Ll = flags_bits(1),
    GENERAL_CATEGORY_Lt = flags_bits(2),
    GENERAL_CATEGORY_LC = GENERAL_CATEGORY_Lu |
                          GENERAL_CATEGORY_Ll |
                          GENERAL_CATEGORY_Lt,
    GENERAL_CATEGORY_Lm = flags_bits(3),
    GENERAL_CATEGORY_Lo = flags_bits(4),
    GENERAL_CATEGORY_L  = GENERAL_CATEGORY_Lu |
                          GENERAL_CATEGORY_Ll |
                          GENERAL_CATEGORY_Lt |
                          GENERAL_CATEGORY_Lm |
                          GENERAL_CATEGORY_Lo,
    GENERAL_CATEGORY_Mn = flags_bits(5),
    GENERAL_CATEGORY_Mc = flags_bits(6),
    GENERAL_CATEGORY_Me = flags_bits(7),
    GENERAL_CATEGORY_M  = GENERAL_CATEGORY_Mn |
                          GENERAL_CATEGORY_Mc |
                          GENERAL_CATEGORY_Me,
    GENERAL_CATEGORY_Nd = flags_bits(8),
    GENERAL_CATEGORY_Nl = flags_bits(9),
    GENERAL_CATEGORY_No = flags_bits(10),
    GENERAL_CATEGORY_N  = GENERAL_CATEGORY_Nd |
                          GENERAL_CATEGORY_Nl |
                          GENERAL_CATEGORY_No,
    GENERAL_CATEGORY_Pc = flags_bits(11),
    GENERAL_CATEGORY_Pd = flags_bits(12),
    GENERAL_CATEGORY_Ps = flags_bits(13),
    GENERAL_CATEGORY_Pe = flags_bits(14),
    GENERAL_CATEGORY_Pi = flags_bits(15),
    GENERAL_CATEGORY_Pf = flags_bits(16),
    GENERAL_CATEGORY_Po = flags_bits(17),
    GENERAL_CATEGORY_P  = GENERAL_CATEGORY_Pc |
                          GENERAL_CATEGORY_Pd |
                          GENERAL_CATEGORY_Ps |
                          GENERAL_CATEGORY_Pe |
                          GENERAL_CATEGORY_Pi |
                          GENERAL_CATEGORY_Pf |
                          GENERAL_CATEGORY_Po,
    GENERAL_CATEGORY_Sm = flags_bits(18),
    GENERAL_CATEGORY_Sc = flags_bits(19),
    GENERAL_CATEGORY_Sk = flags_bits(20),
    GENERAL_CATEGORY_So = flags_bits(21),
    GENERAL_CATEGORY_S  = GENERAL_CATEGORY_Sm |
                          GENERAL_CATEGORY_Sc |
                          GENERAL_CATEGORY_Sk |
                          GENERAL_CATEGORY_So,
    GENERAL_CATEGORY_Zs = flags_bits(22),
    GENERAL_CATEGORY_Zl = flags_bits(23),
    GENERAL_CATEGORY_Zp = flags_bits(24),
    GENERAL_CATEGORY_Z  = GENERAL_CATEGORY_Zs |
                          GENERAL_CATEGORY_Zl |
                          GENERAL_CATEGORY_Zp,
    GENERAL_CATEGORY_Cc = flags_bits(25),
    GENERAL_CATEGORY_Cf = flags_bits(26),
    GENERAL_CATEGORY_Cs = flags_bits(27),
    GENERAL_CATEGORY_Co = flags_bits(28),
    GENERAL_CATEGORY_Cn = flags_bits(29),
    GENERAL_CATEGORY_C  = GENERAL_CATEGORY_Cc |
                          GENERAL_CATEGORY_Cf |
                          GENERAL_CATEGORY_Cs |
                          GENERAL_CATEGORY_Co |
                          GENERAL_CATEGORY_Cn,

    GENERAL_CATEGORY_COUNT = 38,
    GENERAL_CATEGORY_MAX_FLAG_BIT = 29,
    GENERAL_CATEGORY_MASK = flags_bits(GENERAL_CATEGORY_MAX_FLAG_BIT + 1) - 1

#undef flags_bits
};

[[maybe_unused]]
static constexpr const enum general_category all_cats[] = {
    GENERAL_CATEGORY_Lu,
    GENERAL_CATEGORY_Ll,
    GENERAL_CATEGORY_Lt,
    GENERAL_CATEGORY_Lm,
    GENERAL_CATEGORY_Lo,

    GENERAL_CATEGORY_Mn,
    GENERAL_CATEGORY_Mc,
    GENERAL_CATEGORY_Me,

    GENERAL_CATEGORY_Nd,
    GENERAL_CATEGORY_Nl,
    GENERAL_CATEGORY_No,

    GENERAL_CATEGORY_Pc,
    GENERAL_CATEGORY_Pd,
    GENERAL_CATEGORY_Ps,
    GENERAL_CATEGORY_Pe,
    GENERAL_CATEGORY_Pi,
    GENERAL_CATEGORY_Pf,
    GENERAL_CATEGORY_Po,

    GENERAL_CATEGORY_Sm,
    GENERAL_CATEGORY_Sc,
    GENERAL_CATEGORY_Sk,
    GENERAL_CATEGORY_So,

    GENERAL_CATEGORY_Zs,
    GENERAL_CATEGORY_Zl,
    GENERAL_CATEGORY_Zp,

    GENERAL_CATEGORY_Cc,
    GENERAL_CATEGORY_Cf,
    GENERAL_CATEGORY_Cs,
    GENERAL_CATEGORY_Co,
    GENERAL_CATEGORY_Cn,

    GENERAL_CATEGORY_LC,
    GENERAL_CATEGORY_L,
    GENERAL_CATEGORY_M,
    GENERAL_CATEGORY_N,
    GENERAL_CATEGORY_P,
    GENERAL_CATEGORY_S,
    GENERAL_CATEGORY_Z,
    GENERAL_CATEGORY_C
};
static_assert(ARRAY_SIZE(all_cats) == GENERAL_CATEGORY_COUNT, "all_cats[] size mismatch");

struct parsed_unicode_data
{
    // TODO: Add name and case-related fields
    enum general_category cat_by_cp[CODEPOINT_COUNT];
};

enum error_kind
{
    ERROR_KIND_WARNING     = 42,
    ERROR_KIND_ERROR       = 69,
    ERROR_KIND_FATAL_ERROR = 420
};

struct dynamic_string
{
    char* data;
    size_t length;
    size_t capacity;
};

struct thread_pool_task
{
    struct thread_pool_task* next;
    
    void* (*func)(void*);
    void* args;
    void** result_addr;
    bool* done;
};

struct thread_pool_task_queue
{
    struct thread_pool_task* head;  // Synchronized via thread_pool mtx
    struct thread_pool_task* tail;  // Synchronized via thread_pool mtx
    size_t count;                   // Synchronized via thread_pool mtx
};

struct thread_pool
{
    thrd_t* threads;
    size_t thread_count;

    mtx_t mtx;
    cnd_t cnd;

    struct thread_pool_task_queue* task_queue;

    bool shutdown;
};

struct thread_pool_args
{
    struct thread_pool* pool;
    size_t index;
};

static struct thread_pool* thread_pool_create(size_t thrd_count);
static void thread_pool_destroy(struct thread_pool* pool);
static void thread_pool_enqueue(struct thread_pool* pool, void* (*func)(void*), void* args, void** res_addr, bool* done_addr);
static void thread_pool_wait_task(struct thread_pool* pool, bool* done);

static int thread_pool_func(void* args);

static inline const char* general_category_to_string(enum general_category category);
static inline enum general_category general_category_from_string(const char* category);

static inline struct dynamic_string* strip_comments(struct dynamic_string* string);
static inline struct dynamic_string* shrink_to_fit(struct dynamic_string* string);

[[gnu::format(printf, 1, 2)]]
static void emit_warning(const char* format, ...);
[[gnu::format(printf, 1, 2)]]
static void emit_error(const char* format, ...);
[[gnu::format(printf, 1, 2)]]
static void emit_fatal_error(const char* format, ...);

[[noreturn]] [[gnu::format(printf, 2, 3)]] [[gnu::always_inline]]
static inline void exit_with_error(enum error_kind kind, const char* format, ...);

[[noreturn]]
static void print_usage(FILE* stream, int status, const char* progname);

static const char* get_executable_dir();
static const char* current_unicode_version(const char* opt_unicode_version);
static const char* current_output_dir(const char* opt_output_dir);

// Returns a malloc'd string containing the contents of the file at the given URL
// (whose version is the one provided via argument, or the default one if none provided).
static struct dynamic_string download_unicode_file(const char* base_url, const char* filename);

// Returns a malloc'd `struct parsed_unicode_data`
static struct parsed_unicode_data* parse_unicode_data(struct dynamic_string* unicode_data_file);
static bool dump_unicode_data_cat(enum general_category cat, const struct parsed_unicode_data* parsed);

static const struct option long_opts[] = {
    { "help",            no_argument,       nullptr, 'h' },
    { "unicode-version", required_argument, nullptr, 'u' },
    { "output-dir",      required_argument, nullptr, 'o' },
    { "debug",           no_argument,       nullptr, 'd' },
    { "threads",         required_argument, nullptr, 't' },
    { nullptr,           0,                 nullptr,  0  }
};
static const char* const short_opts = "hu:o:dt:";

static FILE* log_file = nullptr;
static char log_file_path[PATH_MAX] = { 0 };
static atomic_bool want_debug = false;
static atomic_size_t thread_count = (size_t)-1;

static struct thread_pool* tpool = nullptr;

/* alignas(alignof(struct thread_pool_task))
static char thread_pool_task_queue_buffer[64 * sizeof(struct thread_pool_task)]; */

/* static_assert(alignof(thread_pool_task_queue_buffer) == alignof(struct thread_pool_task), "buffer alignment mismatch"); */

static void close_log_file(void)
{
    if (log_file)
        fclose(log_file);
}
static void destroy_thread_pool(void)
{
    if (tpool)
        thread_pool_destroy(tpool);
}

int main(int argc, const char* const* const argv)
{
    CURLcode curlret = curl_global_init(CURL_GLOBAL_ALL);
    if (unlikely(curlret != CURLE_OK))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "curl_global_init() failed: %s\n", curl_easy_strerror(curlret));

    char* argv_cpy[argc];
    for (int i = 0; i < argc; ++i)
        argv_cpy[i] = strdup(argv[i]);

    const char* progname = basename(argv[0]);

    int opt;
    while ((opt = getopt_long(argc, argv_cpy, short_opts, long_opts, nullptr)) != -1)
    {
        switch (opt)
        {
            case 'd':
                want_debug = true;
                break;
            case 'h':
                print_usage(stdout, EXIT_SUCCESS, progname);
                break;
            case 'o':
                current_output_dir(optarg);
                break;
            case 't':
                thread_count = strtoul(optarg, nullptr, 10);
                if (unlikely(thread_count == 0))
                    exit_with_error(ERROR_KIND_FATAL_ERROR, "invalid thread count: 0\n");
                break;
            case 'u':
                current_unicode_version(optarg);
                break;
            default:
                print_usage(stderr, EXIT_FAILURE, progname);
                break;
        }
    }

    for (int i = 0; i < argc; ++i)
        free(argv_cpy[i]);

    if (optind < argc)
    {
        emit_error("unexpected argument: %s\n", argv[optind]);
        print_usage(stderr, EXIT_FAILURE, progname);
    }
    

    if (want_debug)
    {
        printf("Thread count: %zu\n", thread_count);
        strcpy(log_file_path, P_tmpdir);
        strcat(log_file_path, "/unistreamslog_XXXXXX");

        int fd = mkstemp(log_file_path);
        if (unlikely(fd == -1))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to create temporary log file\n");
        else
            printf("Log file: %s\n", log_file_path);

        log_file = fdopen(fd, "w");
        if (unlikely(!log_file))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to create temporary log file\n");

        int success = atexit(&close_log_file);
        if (unlikely(success != 0))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to register atexit() function\n");
    }

    if (thread_count == (size_t)-1)
    {
        const size_t usr_nprocs = sysconf(_SC_NPROCESSORS_ONLN);
        thread_count = usr_nprocs > 1 ? usr_nprocs / 2 : 1;
    }
    else
    {
        if (want_debug)
            printf("Using %zu threads\n", thread_count);
    }

    tpool = thread_pool_create(thread_count - 1);
    if (unlikely(thread_count > 1 && !tpool))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to create thread pool\n");
    
    int success = atexit(&destroy_thread_pool);
    if (unlikely(success != 0))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to register atexit() function\n");

    // TODO: Also download:
    // - Files related to case algorithms
    // - Files needed to build POSIX properties as defined here (https://www.unicode.org/reports/tr18/#Compatibility_Properties)
    //   (Maybe also build the equivalent Unicode properties)
    // - Files needed to determine the name of a codepoint (not the most important)
    struct dynamic_string string = download_unicode_file("https://www.unicode.org/Public", "UnicodeData.txt");
    if (unlikely(!string.data) || unlikely(!string.length))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to download UnicodeData.txt\n");

    struct parsed_unicode_data* parsed = parse_unicode_data(shrink_to_fit(&string));

    if (unlikely(!parsed))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to parse UnicodeData.txt\n");

    printf("Generating files...\n");
    // TODO: Dispatch tasks to thread pool, and provide compressed output
    for (size_t i = 0; i < ARRAY_SIZE(all_cats); ++i)
    {
        enum general_category cat = all_cats[i];
        bool ok = dump_unicode_data_cat(cat, parsed);
        if (unlikely(!ok))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to dump UnicodeData.txt for category %s\n", general_category_to_string(cat));
    }

    free(parsed);
    free(string.data);

    curl_global_cleanup();
    exit(EXIT_SUCCESS);
    unreachable();
}

[[noreturn]]
static void print_usage(FILE* stream, int status, const char* progname)
{
    fprintf(stream, "Usage: %s [OPTION]...\n", progname);
    fprintf(stream, "Generate Unicode data files.\n\n");
    fprintf(stream, "Options:\n");
    fprintf(stream, "  -h, --help            display this help and exit\n");
    fprintf(stream, "  -u, --unicode-version=VERSION\n");
    fprintf(stream, "                        specify the Unicode version\n");
    fprintf(stream, "                        (default: %s)\n", current_unicode_version(nullptr));
    fprintf(stream, "  -o, --output-dir=DIR  specify the output directory\n");
    fprintf(stream, "                        (default: %s)\n", current_output_dir(nullptr));
    exit(status);
}

static const char* get_executable_dir()
{
    static bool retrieved = false;
    static char executable_dir[PATH_MAX] = { 0 };

    static mtx_t mtx;
    static once_flag once = ONCE_FLAG_INIT;

    void init_once(void)
    {
        if (unlikely(mtx_init(&mtx, mtx_plain) != thrd_success))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to initialize mutex\n");
    };
    call_once(&once, &init_once);

    if (atomic_load_explicit(&retrieved, memory_order_acquire))
        return executable_dir;

    if (unlikely(mtx_lock(&mtx) != thrd_success))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to lock mutex\n");

    ssize_t len = readlink("/proc/self/exe", executable_dir, ARRAY_SIZE(executable_dir) - 1);
    if (len == -1)
    {
        mtx_unlock(&mtx);
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to readlink: %s\n", strerror(errno));
    }
    executable_dir[len] = '\0';
    char* last_slash = strrchr(executable_dir, '/');
    if (last_slash)
        *last_slash = '\0';

    if (unlikely(mtx_unlock(&mtx) != thrd_success))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to unlock mutex\n");

    atomic_store_explicit(&retrieved, true, memory_order_release);
    return executable_dir;
}

static const char* current_unicode_version(const char* opt_unicode_version)
{
    static char version[32] = DEFAULT_UNICODE_VERSION;

    static mtx_t mtx;
    static once_flag once = ONCE_FLAG_INIT;

    void init_once(void)
    {
        if (unlikely(mtx_init(&mtx, mtx_plain) != thrd_success))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to initialize mutex\n");
    };
    call_once(&once, &init_once);
    
    if (opt_unicode_version)
    {
        if (unlikely(mtx_lock(&mtx) != thrd_success))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to lock mutex\n");

        strncpy(version, opt_unicode_version, ARRAY_SIZE(version) - 1);

        if (unlikely(mtx_unlock(&mtx) != thrd_success))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to unlock mutex\n");
    }
    return version;
}

static const char* current_output_dir(const char* opt_output_dir)
{
    // Default to get_executable_dir() + "/" + DEFAULT_OUTPUT_DIR
    static bool retrieved = false;
    static char output_dir[PATH_MAX] = { 0 };
    
    static mtx_t mtx;
    static once_flag once = ONCE_FLAG_INIT;

    void init_once(void)
    {
        if (unlikely(mtx_init(&mtx, mtx_plain) != thrd_success))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to initialize mutex\n");
    };
    call_once(&once, &init_once);
    
    if (opt_output_dir)
    {
        char* real_path = realpath(opt_output_dir, nullptr);
     
        if (unlikely(!real_path))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to get realpath: %s\n", strerror(errno));

        if (unlikely(mtx_lock(&mtx) != thrd_success))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to lock mutex\n");

        strncpy(output_dir, real_path, ARRAY_SIZE(output_dir) - 1);
        
        if (unlikely(mtx_unlock(&mtx) != thrd_success))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to unlock mutex\n");

        free(real_path);
    }
    else if (!atomic_load_explicit(&retrieved, memory_order_acquire))
    {
        const char* executable_dir = get_executable_dir();
        if (unlikely(!executable_dir))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to get executable directory\n");
        if (unlikely(strlen(executable_dir) + strlen(DEFAULT_OUTPUT_DIR) + 6 > PATH_MAX))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "path too long\n");

        if (unlikely(mtx_lock(&mtx) != thrd_success))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to lock mutex\n");

        snprintf(output_dir, ARRAY_SIZE(output_dir), "%s/%s", executable_dir, DEFAULT_OUTPUT_DIR);

        if (unlikely(mtx_unlock(&mtx) != thrd_success))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to unlock mutex\n");

    }
    
    atomic_store_explicit(&retrieved, true, memory_order_release);
    return output_dir;
}

[[noreturn]] [[gnu::format(printf, 2, 3)]] [[gnu::always_inline]]
static inline void exit_with_error(enum error_kind kind, const char* format, ...)
{
    // Frist, flush any pending output on any streams
    sync();
    switch (kind)
    {
        case ERROR_KIND_WARNING:
            emit_warning(format, __builtin_va_arg_pack ());
            break;
        case ERROR_KIND_ERROR:
            emit_error(format, __builtin_va_arg_pack ());
            break;
        case ERROR_KIND_FATAL_ERROR:
            emit_fatal_error(format, __builtin_va_arg_pack ());
            break;
        default:
            emit_fatal_error("unknown error kind: %d\n", kind);
    }
    exit(kind);
}

[[gnu::format(printf, 1, 2)]]
static void emit_warning(const char* format, ...)
{
    static const char yellow[] = "\033[33m";
    static const char reset[]  = "\033[0m";

    fprintf(stderr, "%swarning%s: ", yellow, reset);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

[[gnu::format(printf, 1, 2)]]
static void emit_error(const char* format, ...)
{
    static const char red[]   = "\033[31m";
    static const char reset[] = "\033[0m";

    fprintf(stderr, "%serror%s: ", red, reset);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

[[gnu::format(printf, 1, 2)]]
static void emit_fatal_error(const char* format, ...)
{
    static const char red_bold[] = "\033[1;31m";
    static const char reset[]    = "\033[0m";

    fprintf(stderr, "%sfatal error%s: ", red_bold, reset);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

static struct dynamic_string download_unicode_file(const char* base_url, const char* filename)
{
    size_t write_callback(void* ptr, size_t size, size_t nmemb, void* userdata)
    {
        // Use reallocarray() and reallocate only when needed (by pagesize elems)
        struct dynamic_string* data = (struct dynamic_string*)userdata;
        size_t new_length = data->length + size * nmemb;
        if (new_length > data->capacity)
        {
            size_t new_capacity = data->capacity + sysconf(_SC_PAGESIZE) + (size * nmemb);
            char* new_data = reallocarray(data->data, new_capacity, sizeof(char));
            if (unlikely(!new_data))
                exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to reallocate memory\n");
            data->data = new_data;
            data->capacity = new_capacity;
        }
        memcpy(data->data + data->length, ptr, size * nmemb);
        data->length = new_length;
        return size * nmemb;
    };

    size_t pagesize = sysconf(_SC_PAGESIZE);
    struct dynamic_string data = {
        .data = calloc(pagesize, sizeof(char)),
        .length = 0,
        .capacity = pagesize
    };
    if (unlikely(!data.data))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to allocate memory\n");

    CURLcode res;

    CURL* curl = curl_easy_init();
    if (unlikely(!curl))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to initialize curl\n");

    char* url = nullptr;
    int retsize = asprintf(&url, "%s/%s/ucd/%s", base_url, current_unicode_version(nullptr), filename);
    if (unlikely(!url) || unlikely(retsize == -1))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to allocate memory\n");
    else
        printf("Downloading %s\n", url);

    res = curl_easy_setopt(curl, CURLOPT_URL, url);
    if (unlikely(res != CURLE_OK))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "curl_easy_setopt() failed: %s\n", curl_easy_strerror(res));

    res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_callback);
    if (unlikely(res != CURLE_OK))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "curl_easy_setopt() failed: %s\n", curl_easy_strerror(res));

    res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&data);
    if (unlikely(res != CURLE_OK))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "curl_easy_setopt() failed: %s\n", curl_easy_strerror(res));

    res = curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    res = curl_easy_perform(curl);
    if (unlikely(res != CURLE_OK))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    else
        printf("Successfully downloaded %ld bytes from %s\n", data.length, url);

    curl_easy_cleanup(curl);
    free(url);

    return data;
}

static inline struct dynamic_string* strip_comments(struct dynamic_string* string)
{
    size_t new_length = 0;
    for (size_t i = 0; i < string->length; ++i)
    {
        if (string->data[i] == '#')
        {
            while (i < string->length && string->data[i] != '\n')
                ++i;
        }
        else
        {
            string->data[new_length] = string->data[i];
            ++new_length;
        }
    }
    string->length = new_length;
    return string;
}

static inline struct dynamic_string* shrink_to_fit(struct dynamic_string* string)
{
    if (likely(string->length < string->capacity))
    {
        char* new_data = reallocarray(string->data, string->length, sizeof(char));
        if (unlikely(!new_data))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to reallocate memory\n");
        string->data = new_data;
        string->capacity = string->length;
    }
    return string;
}

static inline const char* general_category_to_string(enum general_category category)
{
    switch (category & GENERAL_CATEGORY_MASK)
    {
        case GENERAL_CATEGORY_Lu: return "Lu";
        case GENERAL_CATEGORY_Ll: return "Ll";
        case GENERAL_CATEGORY_Lt: return "Lt";
        case GENERAL_CATEGORY_LC: return "LC";
        case GENERAL_CATEGORY_Lm: return "Lm";
        case GENERAL_CATEGORY_Lo: return "Lo";
        case GENERAL_CATEGORY_L:  return "L";
        case GENERAL_CATEGORY_Mn: return "Mn";
        case GENERAL_CATEGORY_Mc: return "Mc";
        case GENERAL_CATEGORY_Me: return "Me";
        case GENERAL_CATEGORY_M:  return "M";
        case GENERAL_CATEGORY_Nd: return "Nd";
        case GENERAL_CATEGORY_Nl: return "Nl";
        case GENERAL_CATEGORY_No: return "No";
        case GENERAL_CATEGORY_N:  return "N";
        case GENERAL_CATEGORY_Pc: return "Pc";
        case GENERAL_CATEGORY_Pd: return "Pd";
        case GENERAL_CATEGORY_Ps: return "Ps";
        case GENERAL_CATEGORY_Pe: return "Pe";
        case GENERAL_CATEGORY_Pi: return "Pi";
        case GENERAL_CATEGORY_Pf: return "Pf";
        case GENERAL_CATEGORY_Po: return "Po";
        case GENERAL_CATEGORY_P:  return "P";
        case GENERAL_CATEGORY_Sm: return "Sm";
        case GENERAL_CATEGORY_Sc: return "Sc";
        case GENERAL_CATEGORY_Sk: return "Sk";
        case GENERAL_CATEGORY_So: return "So";
        case GENERAL_CATEGORY_S:  return "S";
        case GENERAL_CATEGORY_Zs: return "Zs";
        case GENERAL_CATEGORY_Zl: return "Zl";
        case GENERAL_CATEGORY_Zp: return "Zp";
        case GENERAL_CATEGORY_Z:  return "Z";
        case GENERAL_CATEGORY_Cc: return "Cc";
        case GENERAL_CATEGORY_Cf: return "Cf";
        case GENERAL_CATEGORY_Cs: return "Cs";
        case GENERAL_CATEGORY_Co: return "Co";
        case GENERAL_CATEGORY_Cn: return "Cn";
        case GENERAL_CATEGORY_C:  return "C";
        default: return "Unknown or unsupported combination of general categories";
    }
}

static inline enum general_category general_category_from_string(const char* category)
{
#undef  GENERAL_CATEGORY_CASE
#define GENERAL_CATEGORY_CASE(cat)      \
    if (strstr(category, #cat))         \
        return GENERAL_CATEGORY_##cat

    GENERAL_CATEGORY_CASE(Lu);
    GENERAL_CATEGORY_CASE(Ll);
    GENERAL_CATEGORY_CASE(Lt);
    GENERAL_CATEGORY_CASE(LC);
    GENERAL_CATEGORY_CASE(Lm);
    GENERAL_CATEGORY_CASE(Lo);

    GENERAL_CATEGORY_CASE(Mn);
    GENERAL_CATEGORY_CASE(Mc);
    GENERAL_CATEGORY_CASE(Me);

    GENERAL_CATEGORY_CASE(Nd);
    GENERAL_CATEGORY_CASE(Nl);
    GENERAL_CATEGORY_CASE(No);

    GENERAL_CATEGORY_CASE(Pc);
    GENERAL_CATEGORY_CASE(Pd);
    GENERAL_CATEGORY_CASE(Ps);
    GENERAL_CATEGORY_CASE(Pe);
    GENERAL_CATEGORY_CASE(Pi);
    GENERAL_CATEGORY_CASE(Pf);
    GENERAL_CATEGORY_CASE(Po);

    GENERAL_CATEGORY_CASE(Sm);
    GENERAL_CATEGORY_CASE(Sc);
    GENERAL_CATEGORY_CASE(Sk);
    GENERAL_CATEGORY_CASE(So);

    GENERAL_CATEGORY_CASE(Zs);
    GENERAL_CATEGORY_CASE(Zl);
    GENERAL_CATEGORY_CASE(Zp);

    GENERAL_CATEGORY_CASE(Cc);
    GENERAL_CATEGORY_CASE(Cf);
    GENERAL_CATEGORY_CASE(Cs);
    GENERAL_CATEGORY_CASE(Co);
    GENERAL_CATEGORY_CASE(Cn);

    GENERAL_CATEGORY_CASE(L);
    GENERAL_CATEGORY_CASE(M);
    GENERAL_CATEGORY_CASE(N);
    GENERAL_CATEGORY_CASE(P);
    GENERAL_CATEGORY_CASE(S);
    GENERAL_CATEGORY_CASE(Z);
    GENERAL_CATEGORY_CASE(C);

    return GENERAL_CATEGORY_Cn;

#undef GENERAL_CATEGORY_CASE
}

struct unicode_data_lines
{
    char** lines;
    size_t count;
};

struct parse_unicode_data_thread_func_args
{
    size_t cp_range[2];
    struct parsed_unicode_data* parsed;
    const struct unicode_data_lines* unicode_data_lines;
    size_t thread_id;
};

static size_t char_count(const char* restrict str, size_t len, char c)
{
    size_t count = 0;
    for (size_t i = 0; i < len; ++i)
        if (str[i] == c)
            ++count;
    return count;
}

static struct unicode_data_lines* unicode_data_split_lines(const struct dynamic_string* unicode_data_file)
{
    struct unicode_data_lines* lines = malloc(sizeof(struct unicode_data_lines));
    if (unlikely(!lines))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to allocate memory\n");

    size_t naive_count = char_count(unicode_data_file->data, unicode_data_file->length, '\n') + 1;
    lines->count = 0;
    lines->lines = malloc(naive_count * sizeof(char*));
    if (unlikely(!lines->lines))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to allocate memory\n");

    size_t start = 0;
    for (size_t i = strspn(unicode_data_file->data, "\n\r\t\v\f "); i < unicode_data_file->length;)
    {
        if (unicode_data_file->data[i] == '\n')
        {
            size_t len = i - start;
            lines->lines[(lines->count)] = malloc(len + 1);
            if (unlikely(!lines->lines[(lines->count)]))
                exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to allocate memory\n");
            
            memcpy(lines->lines[(lines->count)], unicode_data_file->data + start, len);
            lines->lines[(lines->count)][len] = '\0';
            
            size_t nextl_off = strspn(unicode_data_file->data + i, "\n\r\t\v\f ");
            start = i + nextl_off;
            ++(lines->count);
            
            i += nextl_off;
        }
        else
            ++i;
    }

    void* new = reallocarray(lines->lines, lines->count, sizeof(char*));
    if (unlikely(!new))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to reallocate memory\n");
    lines->lines = new;

    if (want_debug)
    {
        fprintf(log_file, "unicode_data_split_lines():\n");
        fprintf(log_file, "  naive_count: %zu\n", naive_count);
        fprintf(log_file, "  lines->count: %zu\n", lines->count);
        fprintf(log_file, "  expected count: 34931\n");
        if (lines->count != 34931)
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to split UnicodeData.txt\n");
    }

    return lines;
}

static void unicode_data_lines_deallocate(struct unicode_data_lines* lines)
{
    for (size_t i = 0; i < lines->count; ++i)
        if (lines->lines[i])
            free(lines->lines[i]);
    free(lines->lines);
    free(lines);
}

static size_t unicode_data_parse_cp(const char* const delimited_token)
{
    char* tail = nullptr;
    size_t codepoint = strtoull(delimited_token, &tail, 16);
    if (unlikely(*tail != '\0'))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to parse UnicodeData.txt\n");
    return codepoint;
}

static enum general_category unicode_data_parse_cat(const char* const delimited_token)
{
    return general_category_from_string(delimited_token);
}

static bool unicode_data_matches_range_start(const char* const token)
{
    // ".+,\s*First.*" (case-insensitive)
    const char* const regex = ".+,[[:space:]]*First.*";
    regex_t preg;
    regcomp(&preg, regex, REG_ICASE | REG_EXTENDED);
    int ret = regexec(&preg, token, 0, nullptr, 0);
    regfree(&preg);
    return ret == 0;
}
static bool unicode_data_matches_range_end(const char* const token)
{
    // ".+,\s*Last.*" (case-insensitive)
    const char* const regex = ".+,[[:space:]]*Last.*";
    regex_t preg;
    regcomp(&preg, regex, REG_ICASE | REG_EXTENDED);
    int ret = regexec(&preg, token, 0, nullptr, 0);
    regfree(&preg);
    return ret == 0;
}

static thread_local char* processing_buffer_base_ptr = nullptr;
static thread_local char* processing_buffer = nullptr;
static thread_local const size_t processing_buffer_size = 1024UL;

#if TEST_SIGSEGV_HANDLER
static struct {
    thrd_t key;
    size_t value;
}* thread_ids_map = nullptr;
size_t thread_ids_map_size = 0;

static void sigsegv_handler(int sig)
{
    if (sig == SIGSEGV)
    {
        size_t val = 0;
        for (size_t i = 0; i < thread_ids_map_size; ++i)
        {
            if (thrd_equal(thread_ids_map[i].key, thrd_current()))
            {
                val = thread_ids_map[i].value;
                break;
            }
        }
        exit_with_error(ERROR_KIND_FATAL_ERROR, "Caught SIGSEGV in thread %zu (n°%zu)\n", thrd_current(), val);
    }
    else
        exit_with_error(ERROR_KIND_FATAL_ERROR, "Caught unexpected signal %d\n", sig);
}
#endif

static char* get_cp_tok(char** linebuf, size_t* field)
{
    char* tok;
    for (; *field < 1; ++(*field))
        tok = strsep(linebuf, ";");
    return tok;
}
static char* get_name_tok(char** linebuf, size_t* field)
{
    char* tok;
    for (; *field < 2; ++(*field))
        tok = strsep(linebuf, ";");
    return tok;
}
static char* get_cat_tok(char** linebuf, size_t* field)
{
    char* tok;
    for (; *field < 3; ++(*field))
        tok = strsep(linebuf, ";");
    return tok;
};
static char* get_unicode_1_name_tok(char** linebuf, size_t* field)
{
    char* tok;
    for (; *field < 11; ++(*field))
        tok = strsep(linebuf, ";");
    return tok;
}
static char* get_iso_comment_tok(char** linebuf, size_t* field)
{
    char* tok;
    for (; *field < 12; ++(*field))
        tok = strsep(linebuf, ";");
    return tok;
}

static enum general_category parse_unicode_lookup_cat(
    const size_t codepoint, const struct unicode_data_lines* unicode_data_lines
)
{
    static thread_local struct
    {
        size_t start;   // inclusive
        size_t end;     // inclusive
        enum general_category cat;
        bool in;
    } range = {
        -1, -1, GENERAL_CATEGORY_Cn, false
    };
    static thread_local size_t restart_from = 0; // A hint to where to start searching

    if (range.in)
    {
        if (range.start <= codepoint && codepoint <= range.end)
            return range.cat;
        else
            range.in = false;
    }

    if (unlikely(!processing_buffer_base_ptr))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "must allocate processing buffer before calling this function\n");

    processing_buffer = processing_buffer_base_ptr;
    size_t cp = 0;
    size_t fieldind = 0;

    size_t line = 0;
    if (line < restart_from && restart_from < unicode_data_lines->count)
    {
        processing_buffer = processing_buffer_base_ptr;
        fieldind = 0;
        strcpy(processing_buffer, unicode_data_lines->lines[restart_from]);

        cp = unicode_data_parse_cp(get_cp_tok(&processing_buffer, &fieldind));

        if (cp < codepoint)
        {
            line = restart_from;

            processing_buffer = processing_buffer_base_ptr;
            fieldind = 0;
            strcpy(processing_buffer, unicode_data_lines->lines[line]);

            cp = unicode_data_parse_cp(get_cp_tok(&processing_buffer, &fieldind));
        }
        else if (cp == codepoint) goto found;
        else goto normal_prologue;
    }
    else
    {
normal_prologue:
        processing_buffer = processing_buffer_base_ptr;
        fieldind = 0;

        strcpy(processing_buffer, unicode_data_lines->lines[line]);

        cp = unicode_data_parse_cp(get_cp_tok(&processing_buffer, &fieldind));
    }

    while (cp < codepoint && line < unicode_data_lines->count)
    {
        processing_buffer = processing_buffer_base_ptr;
        fieldind = 0;
        strcpy(processing_buffer, unicode_data_lines->lines[line]);

        cp = unicode_data_parse_cp(get_cp_tok(&processing_buffer, &fieldind));
        if (cp < codepoint)
            ++line;
    }
    
    if (line >= unicode_data_lines->count)
    {
        processing_buffer = processing_buffer_base_ptr;
        fieldind = 0;
        strcpy(processing_buffer, unicode_data_lines->lines[unicode_data_lines->count - 1]);

        cp = unicode_data_parse_cp(get_cp_tok(&processing_buffer, &fieldind));

        if (cp < codepoint)
        {
            // Special case: last unicode codepoints (non-allocated)
            range.in = true;
            range.start = cp + 1;
            range.end = MAX_CODEPOINT;
            range.cat = GENERAL_CATEGORY_Cn;

            return GENERAL_CATEGORY_Cn;
        }
        else if (cp == codepoint)
            goto found;
        else
            exit_with_error(ERROR_KIND_FATAL_ERROR, "unexpected error\n");
    }
    else if (cp == codepoint)
    {
found:
        // Branch precondition:
        // - cp == codepoint
        // - line < unicode_data_lines->count
        // - fieldind == 1

        // Cases:
        // 1. Is the start of a range --> store the range
        // 2. Is the end of a range --> do nothing
        // 3. Is a single codepoint --> do nothing
        // 4. Error otherwise
        
        char maybe_start[512] = { 0 }; // TODO: Report GCC bug with strdupa()
        strcpy(maybe_start, get_name_tok(&processing_buffer, &fieldind));

        enum general_category cat = unicode_data_parse_cat(get_cat_tok(&processing_buffer, &fieldind));

        strcat(maybe_start, " ");
        strcat(maybe_start, get_unicode_1_name_tok(&processing_buffer, &fieldind));
        strcat(maybe_start, " ");
        strcat(maybe_start, get_iso_comment_tok(&processing_buffer, &fieldind));
        
        bool matches_start = unicode_data_matches_range_start(maybe_start);

        if (matches_start)
        {
            range.in = true;
            range.start = codepoint;
            range.cat = cat;

            // Search for the end of the range
            if (likely(line + 1 < unicode_data_lines->count))
            {
                processing_buffer = processing_buffer_base_ptr;
                fieldind = 0;
                strcpy(processing_buffer, unicode_data_lines->lines[line + 1]);

                size_t next_cp = unicode_data_parse_cp(get_cp_tok(&processing_buffer, &fieldind));
                char maybe_end[512] = { 0 }; /* strdupa(get_name_tok(&processing_buffer, &fieldind)); */
                strcpy(maybe_end, get_name_tok(&processing_buffer, &fieldind));

                enum general_category next_cat = unicode_data_parse_cat(get_cat_tok(&processing_buffer, &fieldind));
                assert(next_cat == cat);

                strcat(maybe_end, " ");
                strcat(maybe_end, get_unicode_1_name_tok(&processing_buffer, &fieldind));
                strcat(maybe_end, " ");
                strcat(maybe_end, get_iso_comment_tok(&processing_buffer, &fieldind));

                bool matches_end = unicode_data_matches_range_end(maybe_end);

                if (likely(matches_end))
                    range.end = next_cp;

                if (line >= 2)
                    restart_from = line - 2;
                else
                    restart_from = 0;

                return cat;
            }

            exit_with_error(ERROR_KIND_FATAL_ERROR, "start of range without end\n");
        }

        if (line >= 2)
            restart_from = line - 2;
        else
            restart_from = 0;
        return cat;
    }
    else if (cp > codepoint)
    {
        // Cases:
        // 1. Is in a range --> search the range and store it
        // 2. Is the start of unallocated codepoints --> store the range
        // 3. Is the end of unallocated codepoints --> do nothing
        // 4. Error otherwise

        restart_from = line - 2;
        return GENERAL_CATEGORY_Cn;
    }
    else
        exit_with_error(ERROR_KIND_FATAL_ERROR, "unexpected error\n");
    unreachable();
}

static inline const char* parse_thread_id(size_t id)
{
    if (id == (size_t)(-1))
        return "main";
    switch (id)
    {
        case 0: return "0";
        case 1: return "1";
        case 2: return "2";
        case 3: return "3";
        case 4: return "4";
        case 5: return "5";
        case 6: return "6";
        case 7: return "7";
        case 8: return "8";
        case 9: return "9";
        case 10: return "10";
        case 11: return "11";
        case 12: return "12";
        case 13: return "13";
        case 14: return "14";
        case 15: return "15";
        case 16: return "16";
        case 17: return "17";
        case 18: return "18";
        case 19: return "19";
        case 20: return "20";
        case 21: return "21";
        case 22: return "22";
        case 23: return "23";
        case 24: return "24";
        case 25: return "25";
        case 26: return "26";
        case 27: return "27";
        case 28: return "28";
        case 29: return "29";
        case 30: return "30";
        case 31: return "31";
        case 32: return "32";
        default: return "<id too large>";
    }
}

static void* parse_unicode_data_thread_func(void* arg)
{
    struct parse_unicode_data_thread_func_args* args = (struct parse_unicode_data_thread_func_args*)arg;

    const size_t cp_range[2] = { args->cp_range[0], args->cp_range[1] };
    assert(cp_range[0] <= cp_range[1]);

    enum general_category* cat_by_cp = args->parsed->cat_by_cp;

    processing_buffer_base_ptr = malloc(processing_buffer_size);
    processing_buffer = processing_buffer_base_ptr;
    if (unlikely(!processing_buffer))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to allocate memory\n");

    for (size_t i = cp_range[0]; i <= cp_range[1]; ++i)
    {            
        *(cat_by_cp + i) = parse_unicode_lookup_cat(i, args->unicode_data_lines);

        if (want_debug)
        {
            fprintf(log_file, "Thread %s: codepoint 0x%zx is %s\n", parse_thread_id(args->thread_id), i, general_category_to_string(*(cat_by_cp + i)));
            fflush(log_file);
        }
    }

    if (want_debug)
    {
        fprintf(log_file, "Thread %s: processed codepoints %zx to %zx\n", parse_thread_id(args->thread_id), cp_range[0], cp_range[1]);
        printf("Thread %s: processed codepoints %zx to %zx\n", parse_thread_id(args->thread_id), cp_range[0], cp_range[1]);
    }

    free(processing_buffer_base_ptr);

    return nullptr;
}

static struct parsed_unicode_data* parse_unicode_data(struct dynamic_string* unicode_data_file)
{
    // Get time
    struct timespec start, end;
    int r = clock_gettime(CLOCK_MONOTONIC, &start);
    if (unlikely(r == -1))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to get time\n");

    const size_t nthreads = thread_count;
    const size_t to_launch = tpool ? atomic_load_explicit(&(tpool->thread_count), memory_order_acquire) : 0;

    if (want_debug)
    {
        fprintf(log_file, "parse_unicode_data(): nthreads = %zu, to_launch = %zu\n", nthreads, to_launch);
        fflush(log_file);
    }

    struct parse_unicode_data_thread_func_args* args = nullptr;

    args = calloc(nthreads, sizeof(struct parse_unicode_data_thread_func_args));
    if (unlikely(!args))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to allocate memory\n");

    struct parsed_unicode_data* parsed_data = malloc(sizeof(struct parsed_unicode_data));
    if (unlikely(!parsed_data))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to allocate memory\n");
    
    if (want_debug)
        memset(parsed_data->cat_by_cp, ~0, sizeof(parsed_data->cat_by_cp));


    struct unicode_data_lines* splited_lines = unicode_data_split_lines(unicode_data_file);
    if (unlikely(!splited_lines))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to split UnicodeData.txt lines\n");

    printf("Parsing UnicodeData.txt with %zu threads\n", nthreads);

    for (size_t i = 0; i < to_launch; ++i)
    {
        // The remaining codepoints will be processed directly into this func

        args[i + 1].thread_id = i;

        args[i + 1].cp_range[0] =  i      * (MAX_CODEPOINT / nthreads);
        args[i + 1].cp_range[1] = (i + 1) * (MAX_CODEPOINT / nthreads) - 1;

        args[i + 1].parsed = parsed_data;
        args[i + 1].unicode_data_lines = splited_lines;
    }
    // Main thread
    if (to_launch > 0)
    {
        args[0].thread_id = -1;
        args[0].cp_range[0] = to_launch * (MAX_CODEPOINT / nthreads);
        args[0].cp_range[1] = (MAX_CODEPOINT % nthreads) + (nthreads * (MAX_CODEPOINT / nthreads));
        args[0].parsed = parsed_data;
        args[0].unicode_data_lines = splited_lines;
    }
    else
    {
        args[0].thread_id = -1;
        args[0].cp_range[0] = 0;
        args[0].cp_range[1] = MAX_CODEPOINT;
        args[0].parsed = parsed_data;
        args[0].unicode_data_lines = splited_lines;
    }
    
    if (want_debug)
    {
        for (size_t i = 0; i < nthreads; ++i)
            fprintf(log_file, "Thread %s: cp_range[0] = 0x%zx, cp_range[1] = 0x%zx\n", parse_thread_id(args[i].thread_id), args[i].cp_range[0], args[i].cp_range[1]);
    }

    assert(args[0].cp_range[1] == MAX_CODEPOINT);

    bool* finished = calloc(nthreads, sizeof(bool));

    for (size_t i = to_launch; i > 0; --i)
    {
        thread_pool_enqueue(tpool, &parse_unicode_data_thread_func, &args[i], nullptr, &finished[i]);
    }
    thrd_yield();

    /* Process the remaining codepoints directly into this func */
    parse_unicode_data_thread_func(&args[0]);
    finished[0] = true;

    if (to_launch > 0)
    {
        for (size_t i = 0; i < nthreads; ++i)
            thread_pool_wait_task(tpool, finished + i);
    }

    r = clock_gettime(CLOCK_MONOTONIC, &end);
    if (unlikely(r == -1))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to get time\n");

    // Make it microseconds
    time_t sec = end.tv_sec - start.tv_sec;
    long nsec = end.tv_nsec - start.tv_nsec;
    double elapsed = sec + nsec / 1e9;
    printf("All threads have finished processing UnicodeData.txt in %.6f seconds\n", elapsed);

    if (want_debug)
    {
        printf("Verifying parsed data...\n");
        bool error = false;
        for (size_t i = 0; i < CODEPOINT_COUNT; ++i)
        {
            if (parsed_data->cat_by_cp[i] == (size_t)(-1))
            {
                fprintf(log_file, "Codepoint 0x%zx has not been processed correctly\n", i);
                error = true;
            }
        }
        if (error)
            exit_with_error(ERROR_KIND_FATAL_ERROR, "some codepoints have not been processed correctly\n");
    }

#if TEST_SIGSEGV_HANDLER
    if (want_debug)
        free(thread_ids_map);
#endif
    /* if (threads)
        free(threads); */
    unicode_data_lines_deallocate(splited_lines);
    free(args);
    free(finished);

    return parsed_data;
}

static bool dump_unicode_data_cat(enum general_category cat, const struct parsed_unicode_data* parsed)
{
    const char* restrict const output_dir = current_output_dir(nullptr);

    char filename[PATH_MAX] = { 0 };
    snprintf(filename, ARRAY_SIZE(filename), "%s/cp_is_%s.dat", output_dir, general_category_to_string(cat));

    if (want_debug)
        printf("Dumping category information for cat: %s, to file: %s\n", general_category_to_string(cat), filename);

    FILE* file = fopen(filename, "wb");
    if (unlikely(!file))
    {
        emit_warning("failed to open file: %s\n", strerror(errno));
        return false;
    }

    char* buffer = malloc(CODEPOINT_COUNT);
    if (unlikely(!buffer))
    {
        emit_warning("failed to allocate memory\n");
        fclose(file);
        return false;
    }

    for (size_t i = 0; i < CODEPOINT_COUNT; ++i)
    {
        if (parsed->cat_by_cp[i] & cat)
            buffer[i] = '1';
        else
            buffer[i] = '0';
    }

    size_t written = fwrite(buffer, sizeof(char), CODEPOINT_COUNT, file);
    if (unlikely(written != CODEPOINT_COUNT))
    {
        emit_warning("failed to write to file: %s\n", strerror(errno));
        fclose(file);
        free(buffer);
        return false;
    }

    fclose(file);
    free(buffer);
    return true;
}

// Forward declare thread_pool_task_queue_pop
static void thread_pool_task_queue_pop(struct thread_pool_task_queue*, struct thread_pool_task*);

static int thread_pool_func(void* args)
{
    struct thread_pool_args* const targs = (struct thread_pool_args*)args;
    struct thread_pool* const pool       = targs->pool;

    while (true)
    {
        mtx_t* const pmtx = &(pool->mtx);
        cnd_t* const pcnd = &(pool->cnd);
        
        if (unlikely(mtx_lock(pmtx) != thrd_success))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to lock mutex\n");
        
        while (pool->task_queue->count < 1 && !pool->shutdown)
        {
            if (unlikely(cnd_wait(pcnd, pmtx) != thrd_success))
                exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to wait on condition variable\n");
        }
        if (unlikely(pool->shutdown))
        {
            if (unlikely(mtx_unlock(pmtx) != thrd_success))
                exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to unlock mutex\n");
            thrd_exit(thrd_success);
        }

        struct thread_pool_task task;
        thread_pool_task_queue_pop(pool->task_queue, &task);

        if (unlikely(mtx_unlock(pmtx) != thrd_success))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to unlock mutex\n");

        if (unlikely(!task.func))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to pop task from queue\n");

        if (task.result_addr)
            *(task.result_addr) = task.func(task.args);
        else
            task.func(task.args);

        if (likely(task.done))
            atomic_store_explicit(task.done, true, memory_order_release);

        if (unlikely(cnd_broadcast(pcnd) != thrd_success))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to broadcast on condition variable\n");
    }
    unreachable();
}

static struct thread_pool_task_queue* thread_pool_task_queue_create(void)
{
    struct thread_pool_task_queue* queue = malloc(sizeof(struct thread_pool_task_queue));
    if (unlikely(!queue))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to allocate memory\n");

    queue->head = nullptr;
    queue->tail = nullptr;
    queue->count = 0;

    return queue;
}
static void thread_pool_task_queue_destroy(struct thread_pool_task_queue* queue)
{
    if (unlikely(!queue))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "queue is null\n");

    struct thread_pool_task* task = queue->head;
    while (task)
    {
        struct thread_pool_task* next = task->next;
        free(task);
        task = next;
    }

    free(queue);
}

static void thread_pool_task_queue_push(struct thread_pool_task_queue* queue, struct thread_pool_task* task)
{
    if (unlikely(!queue) || unlikely(!task))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "thread_pool_task_queue_push(): invalid arguments\n");

    if (queue->count == 0)
    {
        queue->head = task;
        queue->tail = task;
    }
    else
    {
        queue->tail->next = task;
        queue->tail = task;
    }

    ++(queue->count);
}

static void thread_pool_task_queue_pop(struct thread_pool_task_queue* queue, struct thread_pool_task* task)
{
    if (unlikely(!queue) || unlikely(!task))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "thread_pool_task_queue_pop(): invalid arguments\n");

    if (unlikely(queue->count < 1))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "queue is empty\n");

    memcpy(task, queue->head, sizeof(struct thread_pool_task));

    struct thread_pool_task* next = queue->head->next;
    free(queue->head);
    queue->head = next;

    --(queue->count);

    if (queue->count < 1)
    {
        queue->head = nullptr;
        queue->tail = nullptr;
    }
}

static void thread_pool_wait_task(struct thread_pool* pool, bool* done)
{
    if (unlikely(!pool) || unlikely(!done))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "thread_pool_wait_task(): invalid arguments\n");

    if (unlikely(mtx_lock(&(pool->mtx)) != thrd_success))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to lock mutex\n");

    while (!atomic_load_explicit(done, memory_order_acquire))
    {
        if (unlikely(cnd_wait(&(pool->cnd), &(pool->mtx)) != thrd_success))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to wait on condition variable\n");
    }

    if (unlikely(mtx_unlock(&(pool->mtx)) != thrd_success))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to unlock mutex\n");
}

static struct thread_pool* thread_pool_create(size_t thrd_count)
{
    if (thrd_count < 1) return nullptr;

    static struct thread_pool_args args[64] = { 0 };
    static size_t already_launched = 0;
    static mtx_t mtx = { 0 };

    if (unlikely(atomic_load_explicit(&already_launched, memory_order_acquire) + thrd_count > 64))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "too many threads\n");

    if (unlikely(mtx_lock(&mtx) != thrd_success))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to lock mutex\n");

    struct thread_pool* pool = malloc(sizeof(struct thread_pool));
    if (unlikely(!pool))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to allocate memory\n");

    pool->thread_count = thrd_count;
    pool->threads = calloc(thrd_count, sizeof(thrd_t));
    if (unlikely(!pool->threads))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to allocate memory\n");

    if (unlikely(mtx_init(&(pool->mtx), mtx_plain) != thrd_success))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to initialize mutex\n");

    if (unlikely(cnd_init(&(pool->cnd)) != thrd_success))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to initialize condition variable\n");

    pool->task_queue = thread_pool_task_queue_create();
    if (unlikely(!pool->task_queue))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to create task queue\n");

    pool->shutdown = false;

    const size_t new_count = atomic_load_explicit(&already_launched, memory_order_acquire) + thrd_count;
    for (size_t i = atomic_load_explicit(&already_launched, memory_order_acquire); i < new_count; ++i)
    {
        struct thread_pool_args arg = {
            .pool = pool,
            .index = i
        };
        memcpy(args + i, &arg, sizeof(struct thread_pool_args));

        if (unlikely(thrd_create(pool->threads + i, &thread_pool_func, args + i) != thrd_success))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to create thread\n");

        if (want_debug)
            printf("Thread %zu has been created\n", i);
    }

    if (unlikely(mtx_unlock(&mtx) != thrd_success))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to unlock mutex\n");

    atomic_store_explicit(&already_launched, new_count, memory_order_release);

    return pool;
}
static void thread_pool_destroy(struct thread_pool* pool)
{
    if (unlikely(!pool))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "pool is null\n");

    if (unlikely(mtx_lock(&(pool->mtx)) != thrd_success))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to lock mutex\n");

    thread_pool_task_queue_destroy(pool->task_queue);
    pool->shutdown = true;

    if (unlikely(cnd_broadcast(&(pool->cnd)) != thrd_success))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to broadcast condition variable\n");

    if (unlikely(mtx_unlock(&(pool->mtx)) != thrd_success))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to unlock mutex\n");

    for (size_t i = 0; i < pool->thread_count; ++i)
    {
        if (unlikely(thrd_join(pool->threads[i], nullptr) != thrd_success))
            exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to join thread\n");
    }

    mtx_destroy(&(pool->mtx));
    cnd_destroy(&(pool->cnd));

    free(pool->threads);
    free(pool);
}
static void thread_pool_enqueue(struct thread_pool* pool, void* (*func)(void*), void* args, void** res_addr, bool* done_addr)
{
    if (unlikely(!pool) || unlikely(!func))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "thread_pool_enqueue(): invalid arguments\n");

    struct thread_pool_task* task = malloc(sizeof(struct thread_pool_task));
    if (unlikely(!task))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to allocate memory\n");

    task->func        = func;
    task->args        = args;
    task->result_addr = res_addr;
    task->done        = done_addr;

    if (unlikely(mtx_lock(&(pool->mtx)) != thrd_success))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to lock mutex\n");

    thread_pool_task_queue_push(pool->task_queue, task);

    if (unlikely(cnd_signal(&(pool->cnd)) != thrd_success))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to signal condition variable\n");

    if (unlikely(mtx_unlock(&(pool->mtx)) != thrd_success))
        exit_with_error(ERROR_KIND_FATAL_ERROR, "failed to unlock mutex\n");

    if (want_debug)
        printf("Task has been enqueued\n");
}