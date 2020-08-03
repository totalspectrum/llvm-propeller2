/* from Henry Spencer's stringlib */
/* revised by ERS */

#include <string.h>
#include <ctype.h>
#include <compiler.h>
/*
 * strncmp - compare at most n characters of string s1 to s2
 */

int				/* <0 for <, 0 for ==, >0 for > */
_strncasecmp(const char *scan1, const char *scan2, size_t n)
{
	register char c1, c2;
	register long count;

	if (!scan1) {
		return scan2 ? -1 : 0;
	}
	if (!scan2) return 1;
	count = n;
	do {
	  c1 = toupper(*scan1++);
	  c2 = toupper(*scan2++);
	} while (--count >= 0 && c1 && c1 == c2);

	if (count < 0)
		return(0);

	/*
	 * The following case analysis is necessary so that characters
	 * which look negative collate low against normal characters but
	 * high against the end-of-string NUL.
	 */
	if (c1 == c2)
		return(0);
	else if (c1 == '\0')
		return(-1);
	else if (c2 == '\0')
		return(1);
	else
		return(c1 - c2);
}

__weak_alias(strncasecmp, _strncasecmp);
