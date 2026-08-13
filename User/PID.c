#include "main.h"
#include "PID.h"

void PID_Init(PID_t *p)
{
	p->Target = 0;
	p->Actual = 0;
	p->Out = 0;
	p->Error0 = 0;
	p->Error1 = 0;
	p->ErrorInt = 0;
}

void PID_Update(PID_t *p)
{
	p->Error1 = p->Error0;
	p->Error0 = p->Target - p->Actual;

	if (p->Ki != 0)
	{
		p->ErrorInt += p->Error0;
		if (p->Error0 > p->ErrorIntMax) { p->Error0 = p->ErrorIntMax; }
		else if (p->Error0 < p->ErrorIntMin) { p->Error0 = p->ErrorIntMin; }
	}
	else
	{
		p->ErrorInt = 0;
	}

	p->Out = p->Kp * p->Error0
		   + p->Ki * p->ErrorInt
		   - p->Kd * (p->Actual - p->Actual1);	/* 注意误差和实际值符号相反 */

	if (p->Out > 0) { p->Out += p->OutOffset; }		/* 输出偏移 */
	else if (p->Out < 0) { p->Out -= p->OutOffset; }

	if (p->Out > p->OutMax) { p->Out = p->OutMax; }
	if (p->Out < p->OutMin) { p->Out = p->OutMin; }
	p->Actual1 = p->Actual;
}
