DIALOG dlg_find_name[] =
{
   /* (proc)            (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags)     (d1)                 (d2) (dp)                              (dp2) (dp3) */
   { d_shadow_box_proc, 88,  36,  520, 444, 0,   255, 0,    0,          0,                   0,   NULL,                             NULL, NULL },
   { d_button_proc,     100, 68,  248, 28,  0,   255, 0,    D_EXIT,     0,                   0,   (void*)"OK",                      NULL, NULL },
   { d_button_proc,     356, 68,  240, 28,  0,   255, 0,    D_EXIT,     0,                   0,   (void*)"Cancel",                  NULL, NULL },
   { d_list_proc2,      100, 276, 496, 192, 0,   255, 0,    D_EXIT,     0,                   0,   find_name_getter,                 NULL, NULL },
   { d_text_proc,       100, 40,  464, 24,  0,   255, 0,    0,          0,                   0,   (void*)"find name",               NULL, NULL },
   { d_radio_proc,      100, 104, 224, 20,  0,   255, 0,    D_SELECTED, 0,                   0,   (void*)"From Start",              NULL, NULL },
   { d_radio_proc,      356, 104, 224, 20,  0,   255, 0,    0,          0,                   0,   (void*)"From Cursor",             NULL, NULL },
   { d_radio_proc2,     100, 132, 224, 20,  0,   255, 0,    0,          1,                   0,   (void*)"Function definition",     NULL, NULL },
   { d_radio_proc2,     100, 160, 276, 20,  0,   255, 0,    0,          1,                   0,   (void*)"Within current function", NULL, NULL },
   { d_radio_proc2,     100, 188, 232, 20,  0,   255, 0,    D_SELECTED, 1,                   0,   (void*)"all scope",               NULL, NULL },
   { d_check_proc,      416, 160, 188, 24,  0,   255, 0,    D_DISABLED, 1,                   0,   (void*)"reachable",               NULL, NULL },
   { d_edit_proc,       100, 216, 496, 24,  0,   255, 0,    0,          sizeof(search_text), 0,   (void*)search_text,               NULL, NULL },
   { d_check_proc,      100, 244, 240, 24,  0,   255, 0,    0,          1,                   0,   (void*)"whole word",              NULL, NULL },
   { d_check_proc,      360, 244, 236, 24,  0,   255, 0,    0,          1,                   0,   (void*)"Case sensitive",          NULL, NULL },
   { d_button_proc,     356, 128, 240, 28,  0,   0,   0,    0,          0,                   0,   "Grep",                           NULL, NULL },
   { d_button_proc,     360, 188, 236, 24,  0,   0,   0,    0,          0,                   0,   "Grep Dir",                       NULL, NULL },
   { NULL,              0,   0,   0,   0,   0,   0,   0,    0,          0,                   0,   NULL,                             NULL, NULL }
};