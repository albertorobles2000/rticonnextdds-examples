/* Custom compile data */
struct cdata {
    long param;
    int (*eval_func)(long, long);
};

int divide_test(long sample_data, long p) {
    return (sample_data % p == 0);
}

int gt_test(long sample_data, long p) {
    return (p > sample_data);
}

DDS_ReturnCode_t custom_filter_compile_function(void *filter_data,
        void **new_compile_data, const char *expression,
        const struct DDS_StringSeq *parameters,
        const struct DDS_TypeCode *type_code, const char *type_class_name,
        void *old_compile_data) {
    struct cdata* cd;

    /* First free old data, if any */
    if (old_compile_data != NULL)
        free(old_compile_data);
    /* We expect an expression of the form "%0 %1 <var>"
     * where %1 = "divides" or "greater-than"
     * and <var> is an integral member of the msg structure.
     *
     * We don't actually check that <var> has the correct typecode,
     * (or even that it exists!). See example Typecodes to see
     * how to work with typecodes.
     *
     * The compile information is a structure including the first filter
     * parameter (%0) and a function pointer to evaluate the sample
     */

    /* Check form: */
    if (strncmp(expression, "%0 %1 ", 6) != 0)
        goto err;

    if (strlen(expression) < 7)
        goto err;

    /* Check that we have params */
    if (DDS_StringSeq_get_length(parameters) < 2)
        goto err;

    cd = (struct cdata*) malloc(sizeof(struct cdata));
    sscanf(DDS_StringSeq_get(parameters, 0), "%ld", &cd->param);

    if (strcmp(DDS_StringSeq_get(parameters, 1), "greater-than") == 0) {
        cd->eval_func = gt_test;
    } else if (strcmp(DDS_StringSeq_get(parameters, 1), "divides") == 0) {
        cd->eval_func = divide_test;
    } else {
        goto err;
    }

    *new_compile_data = cd;
    return DDS_RETCODE_OK;
    err: printf("CustomFilter: Unable to compile expression '%s'\n",
            expression);
    printf("              with parameters '%s' '%s'\n",
            DDS_StringSeq_get(parameters, 0), DDS_StringSeq_get(parameters, 1));
    *new_compile_data = NULL;
    return DDS_RETCODE_BAD_PARAMETER;
}

/* Called to evaluated each sample */
DDS_Boolean custom_filter_evaluate_function(void *filter_data,
        void* compile_data, const void* sample,
        const struct DDS_FilterSampleInfo * meta_data) {
    struct cdata* cd;
    struct ccf* msg;

    cd = (struct cdata*) compile_data;
    msg = (struct ccf*) sample;

    if (cd->eval_func(msg->x, cd->param))
        return DDS_BOOLEAN_TRUE;
    else
        return DDS_BOOLEAN_FALSE;
}

void custom_filter_finalize_function(void *filter_data, void *compile_data) {
    if (compile_data != NULL)
        free(compile_data);
}

