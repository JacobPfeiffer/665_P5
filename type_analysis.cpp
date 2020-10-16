#include "ast.hpp"
#include "symbol_table.hpp"
#include "errors.hpp"
#include "types.hpp"
#include "name_analysis.hpp"
#include "type_analysis.hpp"
#include <iostream>

namespace holeyc{

TypeAnalysis * TypeAnalysis::build(NameAnalysis * nameAnalysis){
	//To emphasize that type analysis depends on name analysis
	// being complete, a name analysis must be supplied for 
	// type analysis to be performed.
	TypeAnalysis * typeAnalysis = new TypeAnalysis();
	auto ast = nameAnalysis->ast;	
	typeAnalysis->ast = ast;

	ast->typeAnalysis(typeAnalysis);
	if (typeAnalysis->hasError){
		return nullptr;
	}

	return typeAnalysis;

}

void ProgramNode::typeAnalysis(TypeAnalysis * ta){

	//pass the TypeAnalysis down throughout
	// the entire tree, getting the types for
	// each element in turn and adding them
	// to the ta object's hashMap
	for (auto global : *myGlobals){
		global->typeAnalysis(ta,nullptr);
	}

	//The type of the program node will never
	// be needed. We can just set it to VOID
	//(Alternatively, we could make our type 
	// be error if the DeclListNode is an error)
	ta->nodeType(this, BasicType::produce(VOID));
}

void FnDeclNode::typeAnalysis(TypeAnalysis * ta, TypeNode * retType){

	//HINT: you might want to change the signature for
	// typeAnalysis on FnBodyNode to take a second
	// argument which is the type of the current 
	// function. This will help you to know at a 
	// return statement whether the return type matches
	// the current function

	//Note: this function may need extra code

	// loops through statement nodes
	// call getRetTypeNode for fn return type and compare to any return statement nodes types
	for (auto stmt : *myBody){
		stmt->typeAnalysis(ta,myRetType);
	}
}

void StmtNode::typeAnalysis(TypeAnalysis * ta, TypeNode * retType){
	TODO("Implement me in the subclass");
}

void AssignStmtNode::typeAnalysis(TypeAnalysis * ta, TypeNode * retType){
	myExp->typeAnalysis(ta);

	//It can be a bit of a pain to write 
	// "const DataType *" everywhere, so here
	// the use of auto is used instead to tell the
	// compiler to figure out what the subType variable
	// should be
	auto subType = ta->nodeType(myExp);

	// As error returns null if subType is NOT an error type
	// otherwise, it returns the subType itself
	// nullptr casts to boolean false
	if (subType->asError()){
		ta->nodeType(this, subType);
	} else {
		// if error occurs then set AssignStmtNode nodeType to VOID?
		ta->nodeType(this, BasicType::produce(VOID));
	}
}

void ExpNode::typeAnalysis(TypeAnalysis * ta){
	//TODO("Override me in the subclass");
}

void AssignExpNode::typeAnalysis(TypeAnalysis * ta){
	//TODO: Note that this function is incomplete. 
	// and needs additional code

	//Do typeAnalysis on the subexpressions
	myDst->typeAnalysis(ta);
	mySrc->typeAnalysis(ta);

	const DataType * tgtType = ta->nodeType(myDst);
	const DataType * srcType = ta->nodeType(mySrc);

	//While incomplete, this gives you one case for 
	// assignment: if the types are exactly the same
	// it is usually ok to do the assignment. One
	// exception is that if both types are function
	// names, it should fail type analysis
	if(tgtType->asFn()!=nullptr || srcType->asFn()!=nullptr){
	    if(tgtType->asFn()!=nullptr && srcType->asFn()!=nullptr){
		ta->badAssignOpd(myDst->line(), myDst->col());
		ta->badAssignOpd(mySrc->line(), mySrc->col());
		ta->nodeType(this, ErrorType::produce());
	    }
	    else if( tgtType->asFn()!=nullptr ){
		ta->badAssignOpd(myDst->line(), myDst->col());
		ta->nodeType(this, ErrorType::produce());
	    }
	    else{
		ta->badAssignOpd(mySrc->line(), mySrc->col());
		ta->nodeType(this, ErrorType::produce());
	    }
	    return;
	}
	if (tgtType == srcType){
		if (tgtType->validVarType() == false) { //call false for void and fn types
			//call error
		}
		else {
			ta->nodeType(this, tgtType);
		}
		return;
	}
	// TODO will need to adapt this later but for now set AssignExpNode node to to error and don't report if tgtType or srcType is error
	if(tgtType->asError()!=nullptr || srcType->asError()!=nullptr ){
	    ta->nodeType(this, ErrorType::produce());
	    return;
	}
	
	//Some functions are already defined for you to
	// report type errors. Note that these functions
	// also tell the typeAnalysis object that the
	// analysis has failed, meaning that main.cpp
	// will print "Type check failed" at the end
	ta->badAssignOpr(this->line(), this->col());

	//Note that reporting an error does not set the
	// type of the current node, so setting the node
	// type must be done
	ta->nodeType(this, ErrorType::produce());
}

void DeclNode::typeAnalysis(TypeAnalysis * ta, TypeNode * retType){
	//TODO("Override me in the subclass");
}

void VarDeclNode::typeAnalysis(TypeAnalysis * ta, TypeNode * retType){
	// VarDecls always pass type analysis, since they 
	// are never used in an expression. You may choose
	// to type them void (like this), as discussed in class
	ta->nodeType(this, BasicType::produce(VOID));
}

void IDNode::typeAnalysis(TypeAnalysis * ta){
	// IDs never fail type analysis and always
	// yield the type of their symbol (which
	// depends on their definition)
	ta->nodeType(this, this->getSymbol()->getDataType());
}

void IntLitNode::typeAnalysis(TypeAnalysis * ta){
	// IntLits never fail their type analysis and always
	// yield the type INT
	ta->nodeType(this, BasicType::produce(INT));
}

void CharLitNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, BasicType::produce(CHAR));
}

void FalseNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, BasicType::produce(BOOL));
}

void TrueNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, BasicType::produce(BOOL));
}

void StrLitNode::typeAnalysis(TypeAnalysis * ta){
    ta->nodeType(this, PtrType::produce(BasicType::produce(CHAR),1));
}

void DivideNode::typeAnalysis(TypeAnalysis * ta){
	//Do typeAnalysis on the subexpressions
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	// constant containing the type returned from type analysis on both expressions
	const DataType * exp1 = ta->nodeType(myExp1);
	const DataType * exp2 = ta->nodeType(myExp2);

	// base case is you dont throw an error if the types are compatible for division i.e. both are integers 
	// checks that both expression types are the same and both are int type
	if (exp1 == exp2 && exp1->isInt()){
		ta->nodeType(this, exp1);
	}
	// case where exp1 & exp2 are errors
	else if(exp1->asError()!=nullptr && exp2->asError()!=nullptr ){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp1 is an error
	else if(exp1->asError()!=nullptr && exp2->asError()==nullptr ){
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp2 is an error
	else if(exp1->asError()==nullptr && exp2->asError()!=nullptr ){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}

	// if both are not int throw Arithmetic operator applied to incompatible operands
	else if(!exp1->isInt() && !exp2->isInt()){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// first expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(!exp1->isInt() && exp2->isInt() ){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// second expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(exp1->isInt() && !exp2->isInt()){
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void TimesNode::typeAnalysis(TypeAnalysis * ta){
	//Do typeAnalysis on the subexpressions
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	// constant containing the type returned from type analysis on both expressions
	const DataType * exp1 = ta->nodeType(myExp1);
	const DataType * exp2 = ta->nodeType(myExp2);

	// base case is you dont throw an error if the types are compatible for multiplication i.e. both are integers 
	// checks that both expression types are the same and both are int type
	if (exp1 == exp2 && exp1->isInt()){
		ta->nodeType(this, exp1);
	}
	// case where exp1 & exp2 are errors
	else if(exp1->asError()!=nullptr && exp2->asError()!=nullptr ){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp1 is an error
	else if(exp1->asError()!=nullptr && exp2->asError()==nullptr ){
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp2 is an error
	else if(exp1->asError()==nullptr && exp2->asError()!=nullptr ){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}

	// if both are not int throw Arithmetic operator applied to incompatible operands
	else if(!exp1->isInt() && !exp2->isInt()){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// first expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(!exp1->isInt() && exp2->isInt() ){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// second expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(exp1->isInt() && !exp2->isInt()){
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void MinusNode::typeAnalysis(TypeAnalysis * ta){
	//Do typeAnalysis on the subexpressions
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	// constant containing the type returned from type analysis on both expressions
	const DataType * exp1 = ta->nodeType(myExp1);
	const DataType * exp2 = ta->nodeType(myExp2);

	// base case is you dont throw an error if the types are compatible for subtraction i.e. both are integers 
	// checks that both expression types are the same and both are int type
	if (exp1 == exp2 && exp1->isInt()){
		ta->nodeType(this, exp1);
	}
	// case where exp1 & exp2 are errors
	else if(exp1->asError()!=nullptr && exp2->asError()!=nullptr ){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp1 is an error
	else if(exp1->asError()!=nullptr && exp2->asError()==nullptr ){
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp2 is an error
	else if(exp1->asError()==nullptr && exp2->asError()!=nullptr ){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}

	// if both are not int throw Arithmetic operator applied to incompatible operands
	else if(!exp1->isInt() && !exp2->isInt()){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// first expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(!exp1->isInt() && exp2->isInt() ){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// second expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(exp1->isInt() && !exp2->isInt()){
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void PlusNode::typeAnalysis(TypeAnalysis * ta){
	//Do typeAnalysis on the subexpressions
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	// constant containing the type returned from type analysis on both expressions
	const DataType * exp1 = ta->nodeType(myExp1);
	const DataType * exp2 = ta->nodeType(myExp2);

	// base case is you dont throw an error if the types are compatible for addition i.e. both are integers 
	// checks that both expression types are the same and both are int type
	if (exp1 == exp2 && exp1->isInt()){
		ta->nodeType(this, exp1);
	}
	// case where exp1 & exp2 are errors
	else if(exp1->asError()!=nullptr && exp2->asError()!=nullptr ){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp1 is an error
	else if(exp1->asError()!=nullptr && exp2->asError()==nullptr ){
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp2 is an error
	else if(exp1->asError()==nullptr && exp2->asError()!=nullptr ){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}

	// if both are not int throw Arithmetic operator applied to incompatible operands
	else if(!exp1->isInt() && !exp2->isInt()){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// first expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(!exp1->isInt() && exp2->isInt() ){
	    ta->badMathOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// second expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(exp1->isInt() && !exp2->isInt()){
	    ta->badMathOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void NegNode::typeAnalysis(TypeAnalysis * ta){
	myExp->typeAnalysis(ta);

	const DataType * exp1 = ta->nodeType(myExp);

	// base case is you dont throw an error if the type is an int 
	if (exp1->isInt()){
		ta->nodeType(this, exp1);
	}
	// case where exp1 is an error
	else if(exp1->asError()){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where not an int and not an error
	else{
	    ta->badMathOpd(myExp->line(), myExp->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void PostDecStmtNode::typeAnalysis(TypeAnalysis * ta, TypeNode * retType){
	myLVal->typeAnalysis(ta);

	const DataType * lval = ta->nodeType(myLVal);

	// base case is you dont throw an error if the type is an int 
	if (lval->isInt()){
		ta->nodeType(this, lval);
	}
	// expression is a pointer
	else if (lval->isPtr()){
	    ta->badMathOpr(this->line(),this->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where lval is an error
	else if(lval->asError()!=nullptr){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where not an int and not an error
	else{
	    ta->badMathOpd(myLVal->line(), myLVal->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void PostIncStmtNode::typeAnalysis(TypeAnalysis * ta, TypeNode * retType){
	myLVal->typeAnalysis(ta);

	const DataType * lval = ta->nodeType(myLVal);

	// base case is you dont throw an error if the type is an int 
	if (lval->isInt()){
		ta->nodeType(this, lval);
	}
	// expression is a pointer
	else if (lval->isPtr()){
	    ta->badMathOpr(this->line(),this->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where lval is an error
	else if(lval->asError()!=nullptr){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where not an int and not an error
	else{
	    ta->badMathOpd(myLVal->line(), myLVal->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void GreaterEqNode::typeAnalysis(TypeAnalysis * ta){
	//Do typeAnalysis on the subexpressions
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	// constant containing the type returned from type analysis on both expressions
	const DataType * exp1 = ta->nodeType(myExp1);
	const DataType * exp2 = ta->nodeType(myExp2);

	// base case is you dont throw an error if the types are compatible for relation operator i.e. both are integers 
	// checks that both expression types are the same and both are int type
	if (exp1 == exp2 && exp1->isInt()){
		ta->nodeType(this, exp1);
	}
	// case where exp1 & exp2 are errors
	else if(exp1->asError()!=nullptr && exp2->asError()!=nullptr ){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp1 is an error
	else if(exp1->asError()!=nullptr && exp2->asError()==nullptr ){
	    ta->badRelOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp2 is an error
	else if(exp1->asError()==nullptr && exp2->asError()!=nullptr ){
	    ta->badRelOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}

	// if both are not int throw Arithmetic operator applied to incompatible operands
	else if(!exp1->isInt() && !exp2->isInt()){
	    ta->badRelOpd(myExp1->line(), myExp1->col());
	    ta->badRelOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// first expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(!exp1->isInt() && exp2->isInt() ){
	    ta->badRelOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// second expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(exp1->isInt() && !exp2->isInt()){
	    ta->badRelOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void GreaterNode::typeAnalysis(TypeAnalysis * ta){
	//Do typeAnalysis on the subexpressions
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	// constant containing the type returned from type analysis on both expressions
	const DataType * exp1 = ta->nodeType(myExp1);
	const DataType * exp2 = ta->nodeType(myExp2);

	// base case is you dont throw an error if the types are compatible for relation operator i.e. both are integers 
	// checks that both expression types are the same and both are int type
	if (exp1 == exp2 && exp1->isInt()){
		ta->nodeType(this, exp1);
	}
	// case where exp1 & exp2 are errors
	else if(exp1->asError()!=nullptr && exp2->asError()!=nullptr ){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp1 is an error
	else if(exp1->asError()!=nullptr && exp2->asError()==nullptr ){
	    ta->badRelOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp2 is an error
	else if(exp1->asError()==nullptr && exp2->asError()!=nullptr ){
	    ta->badRelOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}

	// if both are not int throw Arithmetic operator applied to incompatible operands
	else if(!exp1->isInt() && !exp2->isInt()){
	    ta->badRelOpd(myExp1->line(), myExp1->col());
	    ta->badRelOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// first expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(!exp1->isInt() && exp2->isInt() ){
	    ta->badRelOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// second expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(exp1->isInt() && !exp2->isInt()){
	    ta->badRelOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void LessEqNode::typeAnalysis(TypeAnalysis * ta){
	//Do typeAnalysis on the subexpressions
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	// constant containing the type returned from type analysis on both expressions
	const DataType * exp1 = ta->nodeType(myExp1);
	const DataType * exp2 = ta->nodeType(myExp2);

	// base case is you dont throw an error if the types are compatible for relation operator i.e. both are integers 
	// checks that both expression types are the same and both are int type
	if (exp1 == exp2 && exp1->isInt()){
		ta->nodeType(this, exp1);
	}
	// case where exp1 & exp2 are errors
	else if(exp1->asError()!=nullptr && exp2->asError()!=nullptr ){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp1 is an error
	else if(exp1->asError()!=nullptr && exp2->asError()==nullptr ){
	    ta->badRelOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp2 is an error
	else if(exp1->asError()==nullptr && exp2->asError()!=nullptr ){
	    ta->badRelOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}

	// if both are not int throw Arithmetic operator applied to incompatible operands
	else if(!exp1->isInt() && !exp2->isInt()){
	    ta->badRelOpd(myExp1->line(), myExp1->col());
	    ta->badRelOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// first expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(!exp1->isInt() && exp2->isInt() ){
	    ta->badRelOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// second expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(exp1->isInt() && !exp2->isInt()){
	    ta->badRelOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void LessNode::typeAnalysis(TypeAnalysis * ta){
	//Do typeAnalysis on the subexpressions
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	// constant containing the type returned from type analysis on both expressions
	const DataType * exp1 = ta->nodeType(myExp1);
	const DataType * exp2 = ta->nodeType(myExp2);

	// base case is you dont throw an error if the types are compatible for relation operator i.e. both are integers 
	// checks that both expression types are the same and both are int type
	if (exp1 == exp2 && exp1->isInt()){
		ta->nodeType(this, exp1);
	}
	// case where exp1 & exp2 are errors
	else if(exp1->asError()!=nullptr && exp2->asError()!=nullptr ){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp1 is an error
	else if(exp1->asError()!=nullptr && exp2->asError()==nullptr ){
	    ta->badRelOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp2 is an error
	else if(exp1->asError()==nullptr && exp2->asError()!=nullptr ){
	    ta->badRelOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}

	// if both are not int throw Arithmetic operator applied to incompatible operands
	else if(!exp1->isInt() && !exp2->isInt()){
	    ta->badRelOpd(myExp1->line(), myExp1->col());
	    ta->badRelOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// first expression is not an int
	// throw Arithmetic operator applied to invalid operand
	else if(!exp1->isInt() && exp2->isInt() ){
	    ta->badRelOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// second expression is not an int
	else if(exp1->isInt() && !exp2->isInt()){
	    ta->badRelOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void OrNode::typeAnalysis(TypeAnalysis * ta){
	//Do typeAnalysis on the subexpressions
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	// constant containing the type returned from type analysis on both expressions
	const DataType * exp1 = ta->nodeType(myExp1);
	const DataType * exp2 = ta->nodeType(myExp2);

	// base case is you dont throw an error if the types are compatible for logical operator i.e. both are bool 
	// checks that both expression types are the same and both are bool type
	if (exp1 == exp2 && exp1->isBool()){
	    ta->nodeType(this, exp1);
	}
	// case where exp1 & exp2 are errors
	else if(exp1->asError()!=nullptr && exp2->asError()!=nullptr ){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp1 is an error
	else if(exp1->asError()!=nullptr && exp2->asError()==nullptr ){
	    ta->badLogicOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp2 is an error
	else if(exp1->asError()==nullptr && exp2->asError()!=nullptr ){
	    ta->badLogicOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}

	else if(!exp1->isBool() && !exp2->isBool()){
	    ta->badLogicOpd(myExp1->line(), myExp1->col());
	    ta->badLogicOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// first expression is not an bool
	else if(!exp1->isBool() && exp2->isBool() ){
	    ta->badLogicOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// second expression is not an bool
	else if(exp1->isBool() && !exp2->isBool()){
	    ta->badLogicOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void AndNode::typeAnalysis(TypeAnalysis * ta){
	//Do typeAnalysis on the subexpressions
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	// constant containing the type returned from type analysis on both expressions
	const DataType * exp1 = ta->nodeType(myExp1);
	const DataType * exp2 = ta->nodeType(myExp2);

	// base case is you dont throw an error if the types are compatible for logical operator i.e. both are bool 
	// checks that both expression types are the same and both are bool type
	if (exp1 == exp2 && exp1->isBool()){
		ta->nodeType(this, exp1);
	}
	// case where exp1 & exp2 are errors
	else if(exp1->asError()!=nullptr && exp2->asError()!=nullptr ){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp1 is an error
	else if(exp1->asError()!=nullptr && exp2->asError()==nullptr ){
	    ta->badLogicOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp2 is an error
	else if(exp1->asError()==nullptr && exp2->asError()!=nullptr ){
	    ta->badLogicOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}

	else if(!exp1->isBool() && !exp2->isBool()){
	    ta->badLogicOpd(myExp1->line(), myExp1->col());
	    ta->badLogicOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// first expression is not an bool
	else if(!exp1->isBool() && exp2->isBool() ){
	    ta->badLogicOpd(myExp1->line(), myExp1->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// second expression is not an bool
	else if(exp1->isBool() && !exp2->isBool()){
	    ta->badLogicOpd(myExp2->line(), myExp2->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void NotNode::typeAnalysis(TypeAnalysis * ta){
	myExp->typeAnalysis(ta);

	const DataType * exp1 = ta->nodeType(myExp);

	// base case is you dont throw an error if the type is an bool 
	if (exp1->isBool()){
		ta->nodeType(this, exp1);
	}
	// case where exp1 is an error
	else if(exp1->asError()!=nullptr){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where not an bool and not an error
	else{
	    ta->badLogicOpd(myExp->line(), myExp->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void EqualsNode::typeAnalysis(TypeAnalysis * ta){
	//Do typeAnalysis on the subexpressions
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	// constant containing the type returned from type analysis on both expressions
	const DataType * exp1 = ta->nodeType(myExp1);
	const DataType * exp2 = ta->nodeType(myExp2);

	// base case is you dont throw an error if the types are compatible for equality operator i.e. both are same type 
	// checks that both expression types are the same 
	// TODO check to make sure that type is not a function name or of type void
	// case where exp1 & exp2 are errors
	if(exp1->asError()!=nullptr && exp2->asError()!=nullptr ){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp1 is an error
	else if(exp1->asError()!=nullptr && exp2->asError()==nullptr ){
	    ta->badEqOpr(this->line(), this->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp2 is an error
	else if(exp1->asError()==nullptr && exp2->asError()!=nullptr ){
	    ta->badEqOpr(this->line(), this->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	else if (exp1 == exp2){
		ta->nodeType(this, exp1);
	}
	// if both are not the same type throw error
	else{
	    ta->badEqOpr(this->line(), this->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void NotEqualsNode::typeAnalysis(TypeAnalysis * ta){
	//Do typeAnalysis on the subexpressions
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	// constant containing the type returned from type analysis on both expressions
	const DataType * exp1 = ta->nodeType(myExp1);
	const DataType * exp2 = ta->nodeType(myExp2);

	// base case is you dont throw an error if the types are compatible for equality operator i.e. both are same type 
	// case where exp1 & exp2 are errors
	if(exp1->asError()!=nullptr && exp2->asError()!=nullptr ){
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp1 is an error
	else if(exp1->asError()!=nullptr && exp2->asError()==nullptr ){
	    ta->badEqOpr(this->line(), this->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// case where exp2 is an error
	else if(exp1->asError()==nullptr && exp2->asError()!=nullptr ){
	    ta->badEqOpr(this->line(), this->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	// checks that both expression types are the same 
	else if (exp1 == exp2){
		ta->nodeType(this, exp1);
	}
	// if both are not the same type throw error
	else{
	    ta->badEqOpr(this->line(), this->col());
	    ta->nodeType(this, ErrorType::produce());
	}
}

void WhileStmtNode::typeAnalysis(TypeAnalysis * ta, TypeNode * retType){
    // call type Analysis on the condition and on each stmt in the body
	// recursivvely call type analysis on list of StmtNode in the body
	//Do typeAnalysis on the condition
	myCond->typeAnalysis(ta);

	// constant containing the type returned from type analysis on condition
	const DataType * cond = ta->nodeType(myCond);

	if (cond->isBool()){
	    ta->nodeType(this, cond);
	}
	// case where cond is an error
	else if(cond->asError()!=nullptr){
	    ta->nodeType(this, ErrorType::produce());
	}
	// cond is not a bool and not an error type
	else{
	    ta->badWhileCond(myCond->line(), myCond->col());
	    ta->nodeType(this, ErrorType::produce());
	}

	for(auto stmt: *myBody){
	    stmt->typeAnalysis(ta, nullptr);
	}
}

void IfStmtNode::typeAnalysis(TypeAnalysis * ta, TypeNode * retType){
    // call type Analysis on the condition and on each stmt in the body
	// recursivvely call type analysis on list of StmtNode in the body
	//Do typeAnalysis on the condition
	myCond->typeAnalysis(ta);

	// constant containing the type returned from type analysis on condition
	const DataType * cond = ta->nodeType(myCond);

	if (cond->isBool()){
	    ta->nodeType(this, cond);
	}
	// case where cond is an error
	else if(cond->asError()!=nullptr){
	    ta->nodeType(this, ErrorType::produce());
	}
	// cond is not a bool and not an error type
	else{
	    ta->badIfCond(myCond->line(), myCond->col());
	    ta->nodeType(this, ErrorType::produce());
	}

	for(auto stmt: *myBody){
	    stmt->typeAnalysis(ta, nullptr);
	}
}

void IfElseStmtNode::typeAnalysis(TypeAnalysis * ta, TypeNode * retType){
    // call type Analysis on the condition and on each stmt in the body
	// recursivvely call type analysis on list of StmtNode in the body
	//Do typeAnalysis on the condition
	myCond->typeAnalysis(ta);

	// constant containing the type returned from type analysis on condition
	const DataType * cond = ta->nodeType(myCond);

	if (cond->isBool()){
	    ta->nodeType(this, cond);
	}
	// case where cond is an error
	else if(cond->asError()!=nullptr){
	    ta->nodeType(this, ErrorType::produce());
	}
	// cond is not a bool and not an error type
	else{
	    ta->badIfCond(myCond->line(), myCond->col());
	    ta->nodeType(this, ErrorType::produce());
	}

	for(auto stmt: *myBodyTrue){
	    stmt->typeAnalysis(ta, nullptr);
	}

	for(auto stmt: *myBodyFalse){
	    stmt->typeAnalysis(ta, nullptr );
	}
}

void ReturnStmtNode::typeAnalysis(TypeAnalysis * ta, TypeNode * retType){
    // TODO case where return exp is of error type
    // do type analysis on the expression 
	// an empty return stmt
	if(myExp == nullptr){
	    if(!retType->getType()->isVoid()){
		// throw an error return from non void with empty return
		// set node to error type
		ta->badNoRet(this->line(), this->col());
		ta->nodeType(this, ErrorType::produce());
	    }
	}
	// type is void and we return something
	else if(retType->getType()->isVoid()){
	    myExp->typeAnalysis(ta);
	    const DataType * exp = ta->nodeType(myExp);
	    ta->extraRetValue(myExp->line(), myExp->col());
	    ta->nodeType(this, ErrorType::produce());
	}
	else {
	    myExp->typeAnalysis(ta);
	    const DataType * exp = ta->nodeType(myExp);
	    if(exp->asError()!=nullptr){
		ta->nodeType(this, ErrorType::produce());
	    }
	    //type is not void but we return the wrong type
	    else if(exp!=retType->getType()){
		ta->badRetValue(myExp->line(), myExp->col());
		ta->nodeType(this, ErrorType::produce());
	    }
	    //types match
	    else{
		ta->nodeType(this, exp);
	    }
	}
}

void DerefNode::typeAnalysis(TypeAnalysis * ta){ //test
	const DataType* id = ta->nodeType(myID);
	//error if dereferencing a function
	if (id->asFn() != nullptr) {
		ta->fnDeref(myID->line(), myID->col());
		ta->nodeType(this, ErrorType::produce());
	}
	else {
		ta->nodeType(this, id);
	}
}

void RefNode::typeAnalysis(TypeAnalysis * ta){ //test
	const DataType* id = ta->nodeType(myID);	
	ta->nodeType(this, id);
}

void IndexNode::typeAnalysis(TypeAnalysis * ta){ //test
	const DataType* base = ta->nodeType(myBase);
	if (base->isPtr() == false) {
		ta->badPtrBase(myBase->line(), myBase->col());
		ta->nodeType(this, ErrorType::produce());
	}
	else {
		myOffset->typeAnalysis(ta);
		const DataType* offset = ta->nodeType(myOffset);
		if (offset->isInt() == false) {
			ta->badIndex(myBase->line(), myBase->col());
			ta->nodeType(this, ErrorType::produce());
		}
		else {
			ta->nodeType(this, offset);
		}
	}
}

void CallStmtNode::typeAnalysis(TypeAnalysis * ta, TypeNode * myRetType){
	TODO("Implement me in the subclass");
}

void CallExpNode::typeAnalysis(TypeAnalysis * ta){

    // call typeAnalysis on id
    myID->typeAnalysis(ta);
    const DataType * id = ta->nodeType(myID);
    // call to an id that is not a function id
    if(id->asFn()==nullptr){
	ta->badCallee(myID->line(),myID->col());
	ta->nodeType(this, ErrorType::produce());
	return;
    }
    // call type analysis on args
	for(auto argument: *myArgs){
	    argument->typeAnalysis(ta);
	}
    // TODO check error type for arguments
    // wrong # of arguments
    if(id->asFn()->getFormalTypes()->size()!=myArgs->size()){
	ta->badArgCount(myID->line(),myID->col());
    }
    // wrong argument types
    else{
	int i=0;
	int j=0;
	if(id->asFn()->getFormalTypes()!=nullptr){
	    auto it = id->asFn()->getFormalTypes()->begin();
	    for(auto argument: *myArgs){
		    while(j<i){
			it++;
			j++;
		}
		if(ta->nodeType(argument)->asError()!=nullptr){
		}
		else if(ta->nodeType(argument)!=*it ){
		    ta->badArgMatch(argument->line(),argument->col());
		}
		    i++;
	}
    }
		ta->nodeType(this, id->asFn()->getReturnType());
    }
}

void FromConsoleStmtNode::typeAnalysis(TypeAnalysis * ta, TypeNode * retType){
    // call typeAnalysis on myDst
    myDst->typeAnalysis(ta);
    const DataType * dst = ta->nodeType(myDst);
    // myDst is a func
    if(dst->asFn()!=nullptr){
	ta->readFn(myDst->line(),myDst->col());
	ta->nodeType(this, ErrorType::produce());
    }
    // it is a pointer
    else if (dst->isPtr()){
	ta->rawPtr(myDst->line(),myDst->col());
	ta->nodeType(this, ErrorType::produce());
    }
    // TODO make sure that asBasic is correct
    else{
	ta->nodeType(this, dst->asBasic());
    }
}

void ToConsoleStmtNode::typeAnalysis(TypeAnalysis * ta, TypeNode * retType){
    // call typeAnalysis on myDst
    mySrc->typeAnalysis(ta);
    const DataType * src = ta->nodeType(mySrc);
    // myDst is a func
    if(src->asFn()!=nullptr){
	ta->writeFn(mySrc->line(),mySrc->col());
	ta->nodeType(this, ErrorType::produce());
    }
    // it is a pointer
    // TODO allow charptr
    else if (src->isPtr() && src->asPtr()->getString() != "charptr" ){
	ta->rawPtr(mySrc->line(),mySrc->col());
	ta->nodeType(this, ErrorType::produce());
    }
    else if (src->isVoid()){
	ta->badWriteVoid(mySrc->line(),mySrc->col());
	ta->nodeType(this, ErrorType::produce());
    }
    // TODO make sure that asBasic is correct
    else{
	ta->nodeType(this, BasicType::produce(VOID));
    }
}

}
