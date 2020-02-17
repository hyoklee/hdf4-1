/* @(#)xdr.c    1.1 87/11/04 3.9 RPCSRC */
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)xdr.c 1.35 87/08/12";
#endif

#include "H4api_adpt.h"

/*
 * xdr.c, Generic XDR routines implementation.
 *
 * Copyright (C) 1986, Sun Microsystems, Inc.
 *
 * These are the "generic" xdr routines used to serialize and de-serialize
 * most common data items.  See xdr.h for more info on the interface to
 * xdr.
 */

#include <stdio.h>
#include <string.h>

#include "types.h"
#include "xdr.h"

/*
 * constants specific to the xdr "protocol"
 */
#define XDR_FALSE       ((long) 0)
#define XDR_TRUE        ((long) 1)
#define LASTUNSIGNED    ((u_int) 0-1)

/*
 * for unit alignment
 */
static char xdr_zero[BYTES_PER_XDR_UNIT] = { 0, 0, 0, 0 };

/*
 * Free a data structure using XDR
 * Not a filter, but a convenient utility nonetheless
 */
void
xdr_free(proc, objp)
    xdrproc_t proc;
    char *objp;
{
    XDR x;

    x.x_op = XDR_FREE;
    (*proc)(&x, objp);
}

/*
 * XDR nothing
 */
bool_t
xdr_void(void /* xdrs, addr */)
    /* XDR *xdrs; */
    /* caddr_t addr; */
{

    return (TRUE);
}

/* XDR 64bit integers */
bool_t
xdr_int64_t (xdrs, lp)
    register XDR *xdrs;
    int64_t *lp;
{
    int32_t t1, t2;

    switch (xdrs->x_op) {
    case XDR_ENCODE:
        t1 = (int32_t) ((*lp) >> 32);
        t2 = (int32_t) (*lp);
        return (XDR_PUTINT32(xdrs, &t1) && XDR_PUTINT32(xdrs, &t2));
    case XDR_DECODE:
        if (!XDR_GETINT32(xdrs, &t1) || !XDR_GETINT32(xdrs, &t2))
            return FALSE;
        *lp = ((int64_t) t1) << 32;
        *lp |= (uint32_t) t2;    /* Avoid sign extension.  */
        return TRUE;
    case XDR_FREE:
        return TRUE;
    default:
        return FALSE;
    }
}

/* XDR 64bit unsigned integers */
bool_t
xdr_uint64_t (xdrs, ulp)
    register XDR *xdrs;
    uint64_t *ulp;
{
    uint32_t t1;
    uint32_t t2;

    switch (xdrs->x_op) {
    case XDR_ENCODE:
        t1 = (uint32_t) ((*ulp) >> 32);
        t2 = (uint32_t) (*ulp);
        return (XDR_PUTINT32 (xdrs, (int32_t *) &t1) && XDR_PUTINT32(xdrs, (int32_t *) &t2));
    case XDR_DECODE:
        if (!XDR_GETINT32(xdrs, (int32_t *) &t1) || !XDR_GETINT32(xdrs, (int32_t *) &t2))
            return FALSE;
        *ulp = ((uint64_t) t1) << 32;
        *ulp |= t2;
        return TRUE;
    case XDR_FREE:
        return TRUE;
    default:
        return FALSE;
    }
}

/*
 * XDR 32-bit integers
 */
bool_t
xdr_int32_t(xdrs, lp)
    register XDR *xdrs;
    int32_t *lp;
{
    switch (xdrs->x_op) {
    case XDR_ENCODE:
        return XDR_PUTINT32 (xdrs, lp);
    case XDR_DECODE:
        return XDR_GETINT32 (xdrs, lp);
    case XDR_FREE:
        return TRUE;
    default:
        return FALSE;
    }
}

/*
 * XDR unsigned 32-bit integers
 */
bool_t
xdr_uint32_t(xdrs, ulp)
    register XDR *xdrs;
    uint32_t *ulp;
{
    switch (xdrs->x_op) {
    case XDR_ENCODE:
        return XDR_PUTINT32 (xdrs, (int32_t *) ulp);
    case XDR_DECODE:
        return XDR_GETINT32 (xdrs, (int32_t *) ulp);
    case XDR_FREE:
        return TRUE;
    default:
        return FALSE;
    }
}

/* XDR 16bit integers */
bool_t
xdr_int16_t (xdrs, ip)
    register XDR *xdrs;
    int16_t *ip;
{
    int32_t t;

    switch (xdrs->x_op) {
    case XDR_ENCODE:
        t = (int32_t) *ip;
        return XDR_PUTINT32 (xdrs, &t);
    case XDR_DECODE:
        if (!XDR_GETINT32 (xdrs, &t))
            return FALSE;
        *ip = (int16_t) t;
        return TRUE;
    case XDR_FREE:
        return TRUE;
    default:
        return FALSE;
    }
}

/* XDR 16bit unsigned integers */
bool_t
xdr_uint16_t (xdrs, uip)
    register XDR *xdrs;
    uint16_t *uip;
{
    uint32_t ut;

    switch (xdrs->x_op) {
    case XDR_ENCODE:
        ut = (uint32_t) *uip;
        return XDR_PUTINT32 (xdrs, (int32_t *) &ut);
    case XDR_DECODE:
        if (!XDR_GETINT32 (xdrs, (int32_t *) &ut))
            return FALSE;
        *uip = (uint16_t) ut;
        return TRUE;
    case XDR_FREE:
        return TRUE;
    default:
        return FALSE;
    }
}

/* XDR 8bit integers */
bool_t
xdr_int8_t (xdrs, sp)
    register XDR *xdrs;
    int8_t *sp;
{
    int32_t t;

    switch (xdrs->x_op) {
    case XDR_ENCODE:
        t = (int32_t) *sp;
        return XDR_PUTINT32 (xdrs, &t);
    case XDR_DECODE:
        if (!XDR_GETINT32 (xdrs, &t))
            return FALSE;
        *sp = (int8_t) t;
        return TRUE;
    case XDR_FREE:
        return TRUE;
    default:
        return FALSE;
    }
}

/* XDR 8bit unsigned integers */
bool_t
xdr_uint8_t (xdrs, usp)
    register XDR *xdrs;
    uint8_t *usp;
{
    uint32_t ut;

    switch (xdrs->x_op) {
    case XDR_ENCODE:
        ut = (uint32_t) *usp;
        return XDR_PUTINT32 (xdrs, (int32_t *) &ut);
    case XDR_DECODE:
        if (!XDR_GETINT32 (xdrs, (int32_t *) &ut))
            return FALSE;
        *usp = (uint8_t) ut;
        return TRUE;
    case XDR_FREE:
        return TRUE;
    default:
        return FALSE;
    }
}

bool_t
xdr_int(xdrs, ip)
    XDR *xdrs;
    int *ip;
{
    return xdr_int32_t(xdrs, ip);
}

bool_t
xdr_u_int(xdrs, up)
    XDR *xdrs;
    u_int *up;
{
    return xdr_uint32_t(xdrs, up);
}

bool_t
xdr_long(xdrs, lp)
    XDR *xdrs;
    long *lp;
{
    return xdr_int32_t(xdrs, lp);
}

bool_t
xdr_u_long(xdrs, ulp)
    XDR *xdrs;
    u_long *ulp;
{
    return xdr_uint32_t(xdrs, ulp);
}

bool_t
xdr_short(xdrs, sp)
    XDR *xdrs;
    short *sp;
{
    return xdr_int16_t(xdrs, sp);
}

bool_t
xdr_u_short(xdrs, usp)
    XDR *xdrs;
    u_short *usp;
{
    return xdr_uint16_t(xdrs, usp);
}

bool_t
xdr_char(xdrs, cp)
    XDR *xdrs;
    char *cp;
{
    return xdr_int8_t(xdrs, cp);
}

bool_t
xdr_u_char(xdrs, ucp)
    XDR *xdrs;
    unsigned char *ucp;
{
    return xdr_int8_t(xdrs, ucp);
}

/*
 * XDR booleans
 */
bool_t
xdr_bool(xdrs, bp)
    register XDR *xdrs;
    bool_t *bp;
{
    int32_t lb;

    switch (xdrs->x_op) {
    case XDR_ENCODE:
        lb = *bp ? XDR_TRUE : XDR_FALSE;
        return (XDR_PUTLONG(xdrs, &lb));

    case XDR_DECODE:
        if (!XDR_GETLONG(xdrs, &lb)) {
            return FALSE;
        }
        *bp = (lb == XDR_FALSE) ? FALSE : TRUE;
        return TRUE;

    case XDR_FREE:
        return TRUE;
    default:
        return FALSE;
    }
}

/*
 * XDR enumerations
 */
bool_t
xdr_enum(xdrs, ep)
    XDR *xdrs;
    enum_t *ep;
{
    enum sizecheck { SIZEVAL };    /* used to find the size of an enum */

    /*
    * enums are treated as ints
    */
    if (sizeof (enum sizecheck) == sizeof (long)) {
        return (xdr_long(xdrs, (long *)ep));
    }
    else if (sizeof (enum sizecheck) == sizeof (int)) {
        return (xdr_int(xdrs, (int *)ep));
    }
    else if (sizeof (enum sizecheck) == sizeof (short)) {
        return (xdr_short(xdrs, (short *)ep));
    }
    else if (sizeof (enum sizecheck) == sizeof (char)) {
        return (xdr_char(xdrs, (char *)ep));
    }
    else {
        return FALSE;
    }
}

/*
 * XDR opaque data
 * Allows the specification of a fixed size sequence of opaque bytes.
 * cp points to the opaque object and cnt gives the byte length.
 */
bool_t
xdr_opaque(xdrs, cp, cnt)
    register XDR *xdrs;
    caddr_t cp;
    register u_int cnt;
{
    register u_int rndup;
    static int crud[BYTES_PER_XDR_UNIT];

    /*
    * if no data we are done
    */
    if (cnt == 0)
        return (TRUE);

    /*
    * round byte count to full xdr units
    */
    rndup = cnt % BYTES_PER_XDR_UNIT;
    if (rndup > 0)
        rndup = BYTES_PER_XDR_UNIT - rndup;

    if (xdrs->x_op == XDR_DECODE) {
        if (!XDR_GETBYTES(xdrs, cp, cnt)) {
            return (FALSE);
        }
        if (rndup == 0)
            return (TRUE);
        return (XDR_GETBYTES(xdrs, crud, rndup));
    }

    if (xdrs->x_op == XDR_ENCODE) {
        if (!XDR_PUTBYTES(xdrs, cp, cnt)) {
            return (FALSE);
        }
        if (rndup == 0)
            return (TRUE);
        return (XDR_PUTBYTES(xdrs, xdr_zero, rndup));
    }

    if (xdrs->x_op == XDR_FREE) {
        return (TRUE);
    }

    return (FALSE);
}

/*
 * XDR counted bytes
 * *cpp is a pointer to the bytes, *sizep is the count.
 * If *cpp is NULL maxsize bytes are allocated
 */
bool_t
xdr_bytes(xdrs, cpp, sizep, maxsize)
    register XDR *xdrs;
    char **cpp;
    register u_int *sizep;
    u_int maxsize;
{
    register char *sp = *cpp;  /* sp is the actual string pointer */
    register u_int nodesize;

    /*
    * first deal with the length since xdr bytes are counted
    */
    if (! xdr_u_int(xdrs, sizep)) {
        return (FALSE);
    }
    nodesize = *sizep;
    if ((nodesize > maxsize) && (xdrs->x_op != XDR_FREE)) {
        return (FALSE);
    }

    /*
    * now deal with the actual bytes
    */
    switch (xdrs->x_op) {

    case XDR_DECODE:
        if (nodesize == 0) {
            return (TRUE);
        }
        if (sp == NULL) {
            *cpp = sp = (char *)mem_alloc(nodesize);
        }
        if (sp == NULL) {
            (void) fprintf(stderr, "xdr_bytes: out of memory\n");
            return (FALSE);
        }
        /* fall into ... */

    case XDR_ENCODE:
        return (xdr_opaque(xdrs, sp, nodesize));

    case XDR_FREE:
        if (sp != NULL) {
            mem_free(sp, nodesize);
            *cpp = NULL;
        }
        return (TRUE);
    }
    return (FALSE);
}

/*
 * Implemented here due to commonality of the object.
 */
bool_t
xdr_netobj(xdrs, np)
    XDR *xdrs;
    struct netobj *np;
{

    return (xdr_bytes(xdrs, &np->n_bytes, &np->n_len, MAX_NETOBJ_SZ));
}

/*
 * XDR a descriminated union
 * Support routine for discriminated unions.
 * You create an array of xdrdiscrim structures, terminated with
 * an entry with a null procedure pointer.  The routine gets
 * the discriminant value and then searches the array of xdrdiscrims
 * looking for that value.  It calls the procedure given in the xdrdiscrim
 * to handle the discriminant.  If there is no specific routine a default
 * routine may be called.
 * If there is no specific or default routine an error is returned.
 */
bool_t
xdr_union(xdrs, dscmp, unp, choices, dfault)
    register XDR *xdrs;
    enum_t *dscmp;        /* enum to decide which arm to work on */
    char *unp;        /* the union itself */
    struct xdr_discrim *choices;    /* [value, xdr proc] for each arm */
    xdrproc_t dfault;    /* default xdr routine */
{
    register enum_t dscm;

    /*
    * we deal with the discriminator;  it's an enum
    */
    if (! xdr_enum(xdrs, dscmp)) {
        return (FALSE);
    }
    dscm = *dscmp;

    /*
    * search choices for a value that matches the discriminator.
    * if we find one, execute the xdr routine for that value.
    */
    for (; choices->proc != NULL_xdrproc_t; choices++) {
        if (choices->value == dscm)
            return ((*(choices->proc))(xdrs, unp, LASTUNSIGNED));
    }

    /*
    * no match - execute the default xdr routine if there is one
    */
    return ((dfault == NULL_xdrproc_t) ? FALSE :
        (*dfault)(xdrs, unp, LASTUNSIGNED));
}


/*
 * Non-portable xdr primitives.
 * Care should be taken when moving these routines to new architectures.
 */


/*
 * XDR null terminated ASCII strings
 * xdr_string deals with "C strings" - arrays of bytes that are
 * terminated by a NULL character.  The parameter cpp references a
 * pointer to storage; If the pointer is null, then the necessary
 * storage is allocated.  The last parameter is the max allowed length
 * of the string as specified by a protocol.
 */
bool_t
xdr_string(xdrs, cpp, maxsize)
    register XDR *xdrs;
    char **cpp;
    u_int maxsize;
{
    register char *sp = *cpp;  /* sp is the actual string pointer */
    u_int size;
    u_int nodesize;

    /*
    * first deal with the length since xdr strings are counted-strings
    */
    switch (xdrs->x_op) {
    case XDR_DECODE:
        break; /* keep gcc happy */
    case XDR_FREE:
        if (sp == NULL) {
            return(TRUE);    /* already free */
        }
        /* fall through... */
    case XDR_ENCODE:
        size = strlen(sp);
        break;
    }
    if (! xdr_u_int(xdrs, &size)) {
        return (FALSE);
    }
    if (size > maxsize) {
        return (FALSE);
    }
    nodesize = size + 1;

    /*
    * now deal with the actual bytes
    */
    switch (xdrs->x_op) {

    case XDR_DECODE:
        if (nodesize == 0) {
            return (TRUE);
        }
        if (sp == NULL)
            *cpp = sp = (char *)mem_alloc(nodesize);
        if (sp == NULL) {
            (void) fprintf(stderr, "xdr_string: out of memory\n");
            return (FALSE);
        }
        sp[size] = 0;
        /* fall into ... */

    case XDR_ENCODE:
        return (xdr_opaque(xdrs, sp, size));

    case XDR_FREE:
        mem_free(sp, nodesize);
        *cpp = NULL;
        return (TRUE);
    }
    return (FALSE);
}

/*
 * Wrapper for xdr_string that can be called directly from
 * routines like clnt_call
 */
bool_t
xdr_wrapstring(xdrs, cpp)
    XDR *xdrs;
    char **cpp;
{
    if (xdr_string(xdrs, cpp, LASTUNSIGNED)) {
        return (TRUE);
    }
    return (FALSE);
}
