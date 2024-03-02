#ifndef f_AT_DEBUGGEREXP_H
#define f_AT_DEBUGGEREXP_H

#include <stdarg.h>
#include <vd2/system/vdalloc.h>
#include <vd2/system/error.h>

class VDStringA;
class ATCPUEmulator;
class ATCPUEmulatorMemory;
class ATAnticEmulator;
class IATDebugger;

enum ATDebugExpNodeType {
	kATDebugExpNodeType_None,
	kATDebugExpNodeType_PC,
	kATDebugExpNodeType_A,
	kATDebugExpNodeType_X,
	kATDebugExpNodeType_Y,
	kATDebugExpNodeType_S,
	kATDebugExpNodeType_P,
	kATDebugExpNodeType_Or,
	kATDebugExpNodeType_And,
	kATDebugExpNodeType_LT,
	kATDebugExpNodeType_LE,
	kATDebugExpNodeType_GT,
	kATDebugExpNodeType_GE,
	kATDebugExpNodeType_NE,
	kATDebugExpNodeType_EQ,
	kATDebugExpNodeType_Add,
	kATDebugExpNodeType_Sub,
	kATDebugExpNodeType_Mul,
	kATDebugExpNodeType_Div,
	kATDebugExpNodeType_DerefByte,
	kATDebugExpNodeType_DerefWord,
	kATDebugExpNodeType_Invert,
	kATDebugExpNodeType_Negate,
	kATDebugExpNodeType_Const,
	kATDebugExpNodeType_Read,
	kATDebugExpNodeType_Write,
	kATDebugExpNodeType_HPOS,
	kATDebugExpNodeType_VPOS
};

struct ATDebugExpEvalContext {
	ATCPUEmulator *mpCPU;
	ATCPUEmulatorMemory *mpMemory;
	ATAnticEmulator *mpAntic;
};

class ATDebugExpNode {
public:
	const ATDebugExpNodeType mType;

	ATDebugExpNode(ATDebugExpNodeType nodeType)
		: mType(nodeType)
	{
	}

	virtual ~ATDebugExpNode() {}

	virtual bool Evaluate(sint32& result, const ATDebugExpEvalContext& context) const = 0;

	/// Attempt to extract out a (node == const) clause; returns the constant and the
	/// remainder expression.
	virtual bool ExtractEqConst(ATDebugExpNodeType type, ATDebugExpNode **extracted, ATDebugExpNode **remainder) { return false; }

	virtual bool Optimize(ATDebugExpNode **result) { return false; }

	virtual void ToString(VDStringA& s) {
		ToString(s, 0);
	}

	virtual void ToString(VDStringA& s, int prec) = 0;
};

ATDebugExpNode *ATDebuggerParseExpression(const char *s, IATDebugger *dbg);
ATDebugExpNode *ATDebuggerInvertExpression(ATDebugExpNode *node);

class ATDebuggerExprParseException : public MyError {
public:
	ATDebuggerExprParseException(const char *s, ...) {
		va_list val;
		va_start(val, s);
		vsetf(s, val);
		va_end(val);
	}
};

#endif