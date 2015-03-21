/*
 * =====================================================================================
 *
 *       Filename:  apb.cpp
 *
 *    Description:  A plus B.
 *
 *        Version:  1.0
 *        Created:  3/21/2015 8:08:50 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  XadillaX (admin@xcoder.in)
 *   Organization:  Ningbo University of Technology
 *
 * =====================================================================================
 */
#include <stdio.h>

int main()
{
    int a, b;
    while(~scanf("%d%d", &a, &b))
    {
        printf("%d\n", a + b);
    }
    return 0;
}

