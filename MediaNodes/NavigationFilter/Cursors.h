const uint8 cursorLeft[] = {
	16,		/* cursor size */
	1,		/* bits per pixel */
	7,		/* vertical hot spot */
	0,		/* horizontal hot spot */

	/* data */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x18, 0x00, 0x30, 0x00, 0x60, 0x00, 0x7f, 0xfe, 
	0x7f, 0xfe, 0x60, 0x00, 0x30, 0x00, 0x18, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	/* mask */
	0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x1e, 0x00, 0x3c, 0x00, 0x78, 0x00, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0x78, 0x00, 0x3c, 0x00, 0x1e, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8 cursorRight[] = {
	16,		/* cursor size */
	1,		/* bits per pixel */
	7,		/* vertical hot spot */
	15,		/* horizontal hot spot */

	/* data */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x18, 0x00, 0x0c, 0x00, 0x06, 0x7f, 0xfe, 
	0x7f, 0xfe, 0x00, 0x06, 0x00, 0x0c, 0x00, 0x18, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	/* mask */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x78, 0x00, 0x3c, 0x00, 0x1e, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0x00, 0x1e, 0x00, 0x3c, 0x00, 0x78, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00
};

const uint8 cursorUp[] = {
	16,		/* cursor size */
	1,		/* bits per pixel */
	0,		/* vertical hot spot */
	7,		/* horizontal hot spot */

	/* data */
	0x00, 0x00, 0x03, 0xc0, 0x07, 0xe0, 0x0d, 0xb0, 0x19, 0x98, 0x11, 0x88, 0x01, 0x80, 0x01, 0x80, 
	0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00,

	/* mask */
	0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0, 0x1f, 0xf8, 0x3f, 0xfc, 0x3b, 0xdc, 0x33, 0xcc, 0x03, 0xc0, 
	0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0
};

const uint8 cursorDown[] = {
	16,		/* cursor size */
	1,		/* bits per pixel */
	15,		/* vertical hot spot */
	7,		/* horizontal hot spot */

	/* data */
	0x00, 0x00, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 
	0x01, 0x80, 0x01, 0x80, 0x11, 0x88, 0x19, 0x98, 0x0d, 0xb0, 0x07, 0xe0, 0x03, 0xc0, 0x00, 0x00,

	/* mask */
	0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 
	0x03, 0xc0, 0x33, 0xcc, 0x3b, 0xdc, 0x3f, 0xfc, 0x1f, 0xf8, 0x0f, 0xf0, 0x07, 0xe0, 0x03, 0xc0
};

const uint8 cursorLeft_Up[] = {
	16,		/* cursor size */
	1,		/* bits per pixel */
	1,		/* vertical hot spot */
	1,		/* horizontal hot spot */

	/* data */
	0x00, 0x00, 0x3f, 0x00, 0x77, 0x00, 0x78, 0x00, 0x5c, 0x00, 0x6e, 0x00, 0x67, 0x00, 0x63, 0x80, 
	0x01, 0xc0, 0x00, 0xe0, 0x00, 0x70, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	/* mask */
	0x7f, 0x00, 0xfe, 0x80, 0xfe, 0x80, 0xff, 0x00, 0xfe, 0x00, 0xff, 0x00, 0xff, 0x80, 0x97, 0xc0, 
	0x63, 0xe0, 0x01, 0xf0, 0x00, 0xf8, 0x00, 0x70, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


const uint8 cursorRight_Up[] = {
	16,		/* cursor size */
	1,		/* bits per pixel */
	1,		/* vertical hot spot */
	14,		/* horizontal hot spot */

	/* data */
	0x00, 0x00, 0x00, 0xfc, 0x00, 0xf6, 0x00, 0x0e, 0x00, 0x3a, 0x00, 0x76, 0x00, 0xe6, 0x01, 0xc6, 
	0x03, 0x80, 0x07, 0x00, 0x0e, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	/* mask */
	0x00, 0xfe, 0x01, 0xff, 0x01, 0xff, 0x00, 0xff, 0x00, 0x7f, 0x00, 0xff, 0x01, 0xff, 0x03, 0xef, 
	0x07, 0xc6, 0x0f, 0x80, 0x1f, 0x00, 0x0e, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8 cursorRightDown[] = {
	16,		/* cursor size */
	1,		/* bits per pixel */
	14,		/* vertical hot spot */
	14,		/* horizontal hot spot */

	/* data */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0e, 0x00, 0x07, 0x00, 0x03, 0x80, 
	0x01, 0xc6, 0x00, 0xe6, 0x00, 0x76, 0x00, 0x3a, 0x00, 0x1e, 0x00, 0xee, 0x00, 0xfc, 0x00, 0x00,

	/* mask */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0e, 0x00, 0x1f, 0x00, 0x0f, 0x80, 0x07, 0xc6, 
	0x03, 0xe9, 0x01, 0xff, 0x00, 0xff, 0x00, 0x7f, 0x00, 0xff, 0x01, 0x7f, 0x01, 0x7f, 0x00, 0xfe
};

const uint8 cursorLeft_Down[] = {
	16,		/* cursor size */
	1,		/* bits per pixel */
	14,		/* vertical hot spot */
	1,		/* horizontal hot spot */

	/* data */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x70, 0x00, 0xe0, 0x01, 0xc0, 
	0x63, 0x80, 0x67, 0x00, 0x6e, 0x00, 0x5c, 0x00, 0x78, 0x00, 0x77, 0x00, 0x3f, 0x00, 0x00, 0x00,

	/* mask */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x70, 0x00, 0xf8, 0x01, 0xf0, 0x63, 0xe0, 
	0x97, 0xc0, 0xff, 0x80, 0xff, 0x00, 0xfe, 0x00, 0xff, 0x00, 0xfe, 0x80, 0xfe, 0x80, 0x7f, 0x00
};
