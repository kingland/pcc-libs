/*	$Id: dfe.c,v 1.2 2008/02/26 19:54:44 ragge Exp $	*/
/*
 * Copyright(C) Caldera International Inc. 2001-2002. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code and documentation must retain the above
 * copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditionsand the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * All advertising materials mentioning features or use of this software
 * must display the following acknowledgement:
 * 	This product includes software developed or owned by Caldera
 *	International, Inc.
 * Neither the name of Caldera International, Inc. nor the names of other
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * USE OF THE SOFTWARE PROVIDED FOR UNDER THIS LICENSE BY CALDERA
 * INTERNATIONAL, INC. AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL CALDERA INTERNATIONAL, INC. BE LIABLE
 * FOR ANY DIRECT, INDIRECT INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OFLIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "fio.h"
#include "fmt.h"
extern int rd_ed(),rd_ned(),y_getc(),y_putc(),y_err();
extern int y_rev();
extern int w_ed(),w_ned();
s_rdfe(a) cilist *a;
{
	int n;
	if(!init) f_init();
	if(n=c_dfe(a,READ))return(n);
	reading=1;
	if(curunit->uwrt) nowreading(curunit);
	getn = y_getc;
	doed = rd_ed;
	doned = rd_ned;
	dorevert = donewrec = y_err;
	doend = y_rev;
	if(pars_f(fmtbuf)<0)
		err(a->cierr,100,"startio");
	fmt_bg();
	return(0);
}
s_wdfe(a) cilist *a;
{
	int n;
	if(!init) f_init();
	if(n=c_dfe(a,WRITE)) return(n);
	reading=0;
	if(!curunit->uwrt) nowwriting(curunit);
	putn = y_putc;
	doed = w_ed;
	doned= w_ned;
	dorevert = donewrec = y_err;
	doend = y_rev;
	if(pars_f(fmtbuf)<0)
		err(a->cierr,100,"startwrt");
	fmt_bg();
	return(0);
}
e_rdfe()
{
	en_fio();
	return(0);
}
e_wdfe()
{
	en_fio();
	return(0);
}
c_dfe(a,flag) cilist *a;
{
	sequential=0;
	formatted=external=1;
	elist=a;
	cursor=scale=recpos=0;
	if(a->ciunit>MXUNIT || a->ciunit<0)
		err(a->cierr,101,"startchk");
	curunit = &units[a->ciunit];
	if(curunit->ufd==NULL && fk_open(flag,DIR,FMT,a->ciunit))
		err(a->cierr,104,"dfe");
	cf=curunit->ufd;
	if(!curunit->ufmt) err(a->cierr,102,"dfe")
	if(!curunit->useek) err(a->cierr,104,"dfe")
	fmtbuf=a->cifmt;
	fseek(cf,(long)curunit->url * (a->cirec-1),0);
	curunit->uend = 0;
	return(0);
}
y_getc()
{
	int ch;
	if(curunit->uend) return(-1);
	if((ch=getc(cf))!=EOF)
	{
		recpos++;
		if(curunit->url>=recpos ||
			curunit->url==1)
			return(ch);
		else	return(' ');
	}
	if(feof(cf))
	{
		curunit->uend=1;
		errno=0;
		return(-1);
	}
	err(elist->cierr,errno,"readingd");
}
y_putc(c)
{
	recpos++;
	if(recpos <= curunit->url || curunit->url==1)
		putc(c,cf);
	else
		err(elist->cierr,110,"dout");
	return(0);
}
y_rev()
{	/*what about work done?*/
	if(curunit->url==1 || recpos==curunit->url)
		return(0);
	while(recpos<curunit->url)
		(*putn)(' ');
	recpos=0;
	return(0);
}
y_err()
{
	err(elist->cierr, 110, "dfe");
}
