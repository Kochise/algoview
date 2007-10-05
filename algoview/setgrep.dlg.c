DIALOG dlg_set_grep[] =
{
   /* (proc)        (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)                           (dp2) (dp3) */
   { d_box_proc,    36,  36,  548, 404, 0,   0,   0,    0,      0,   0,   NULL,                          NULL, NULL },
   { d_button_proc, 416, 72,  148, 36,  0,   0,   0,    0,      0,   0,   (void*)"Select Dir",           NULL, NULL },
   { d_button_proc, 264, 120, 104, 32,  0,   0,   0,    0,      0,   0,   (void*)"Add",                  NULL, NULL },
   { d_button_proc, 372, 120, 96,  32,  0,   0,   0,    0,      0,   0,   (void*)"remove",               NULL, NULL },
   { d_text_proc,   128, 72,  284, 36,  0,   0,   0,    0,      0,   0,   (void*)"text",                 NULL, NULL },
   { d_list_proc,   48,  160, 520, 268, 0,   0,   0,    0,      0,   0,   dummy_getter,                  NULL, NULL },
   { d_edit_proc,   132, 120, 128, 32,  0,   0,   0,    0,      4,   0,   (void*)"edit",                 NULL, NULL },
   { d_text_proc,   52,  120, 68,  32,  0,   0,   0,    0,      0,   0,   (void*)"ext",                  NULL, NULL },
   { d_text_proc,   52,  72,  72,  36,  0,   0,   0,    0,      0,   0,   (void*)"dir",                  NULL, NULL },
   { d_ctext_proc,  48,  40,  520, 24,  0,   0,   0,    0,      0,   0,   (void*)"Set Grep Environment", NULL, NULL },
   { d_button_proc, 472, 120, 100, 32,  0,   0,   0,    0,      0,   0,   (void*)"OK",                   NULL, NULL },
   { NULL,          0,   0,   0,   0,   0,   0,   0,    0,      0,   0,   NULL,                          NULL, NULL }
};