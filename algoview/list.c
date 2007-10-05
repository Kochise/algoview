DIALOG dlg_function_list[] =
{
   /* (proc)            (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)                     (dp2) (dp3) */
   { d_shadow_box_proc, 32,  32,  588, 432, 0,   0,   0,    0,      0,   0,   NULL,                    NULL, NULL },
   { d_button_proc,     56,  416, 260, 32,  0,   0,   0,    0,      0,   0,   (void*)"OK",             NULL, NULL },
   { d_button_proc,     332, 416, 264, 32,  0,   0,   0,    0,      0,   0,   (void*)"Cancel",         NULL, NULL },
   { d_list_proc,       56,  68,  540, 340, 0,   0,   0,    0,      0,   0,   dummy_getter,            NULL, NULL },
   { d_text_proc,       56,  44,  532, 16,  0,   0,   0,    0,      0,   0,   (void*)"Go to function", NULL, NULL },
   { NULL,              0,   0,   0,   0,   0,   0,   0,    0,      0,   0,   NULL,                    NULL, NULL }
};