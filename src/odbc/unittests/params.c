#include "common.h"

/* Test for store procedure and params */
/* Test from Tom Rogers */

static char software_version[] = "$Id: params.c,v 1.7 2005-03-29 15:19:36 freddy77 Exp $";
static void *no_unused_var_warn[] = { software_version, no_unused_var_warn };

/* SP definition */

static const char sp_define[] = "CREATE PROCEDURE spTestProc\n"
	"    @InParam int,\n"
	"    @OutParam int OUTPUT,\n"
	"    @OutString varchar(20) OUTPUT\n"
	"AS\n"
	"     SELECT @OutParam = @InParam\n"
	"     SELECT @OutString = 'This is cool!'\n"
	"     IF @InParam < 10\n" "          RETURN (0)\n" "     ELSE\n" "          RETURN (1)\n";

#define SP_TEXT "{?=call spTestProc(?,?,?)}"
#define OUTSTRING_LEN 20

static int
Test(int bind_before)
{
	SQLSMALLINT ReturnCode = 0;
	SQLSMALLINT InParam = 5;
	SQLSMALLINT OutParam = 1;
	char OutString[OUTSTRING_LEN];
	SQLLEN cbReturnCode = 0, cbInParam = 0, cbOutParam = 0;
	SQLLEN cbOutString = SQL_NTS;

	Connect();

	/* drop proc */
	if (CommandWithResult(Statement, "drop proc spTestProc") != SQL_SUCCESS)
		printf("Unable to execute statement\n");

	/* create proc */
	Command(Statement, sp_define);

	if (!bind_before) {
		if (SQLPrepare(Statement, (SQLCHAR *) SP_TEXT, strlen(SP_TEXT)) != SQL_SUCCESS) {
			fprintf(stderr, "Unable to prepare statement\n");
			return 1;
		}
	}

	if (SQLBindParameter(Statement, 1, SQL_PARAM_OUTPUT, SQL_C_SSHORT, SQL_INTEGER, 0, 0, &ReturnCode, 0, &cbReturnCode) !=
	    SQL_SUCCESS) {
		fprintf(stderr, "Unable to bind input parameter\n");
		return 1;
	}

	if (SQLBindParameter(Statement, 2, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_INTEGER, 0, 0, &InParam, 0, &cbInParam) !=
	    SQL_SUCCESS) {
		fprintf(stderr, "Unable to bind input parameter\n");
		return 1;
	}

	if (SQLBindParameter(Statement, 3, SQL_PARAM_OUTPUT, SQL_C_SSHORT, SQL_INTEGER, 0, 0, &OutParam, 0, &cbOutParam) !=
	    SQL_SUCCESS) {
		fprintf(stderr, "Unable to bind input parameter\n");
		return 1;
	}

	OutString[0] = '\0';
	strcpy(OutString, "Test");	/* Comment this line and we get an error!  Why? */
	if (SQLBindParameter
	    (Statement, 4, SQL_PARAM_OUTPUT, SQL_C_CHAR, SQL_VARCHAR, OUTSTRING_LEN, 0, OutString, OUTSTRING_LEN,
	     &cbOutString) != SQL_SUCCESS) {
		fprintf(stderr, "Unable to bind input parameter\n");
		return 1;
	}

	if (bind_before) {
		if (SQLPrepare(Statement, (SQLCHAR *) SP_TEXT, strlen(SP_TEXT)) != SQL_SUCCESS) {
			fprintf(stderr, "Unable to prepare statement\n");
			return 1;
		}
	}

	if (SQLExecute(Statement) != SQL_SUCCESS) {
		fprintf(stderr, "Unable to execute statement\n");
		return 1;
	}

	Command(Statement, "drop proc spTestProc");

	printf("Output:\n");
	printf("   Return Code = %d\n", (int) ReturnCode);
	printf("   InParam = %d\n", (int) InParam);
	printf("   OutParam = %d\n", (int) OutParam);
	printf("   OutString = %s\n", OutString);

	if (InParam != OutParam) {
		fprintf(stderr, "InParam != OutParam\n");
		return 1;
	}

	if (strcmp(OutString, "This is cool!") != 0) {
		fprintf(stderr, "Bad string returned\n");
		return 1;
	}

	Disconnect();
	return 0;
}

int
main(int argc, char *argv[])
{
	if (Test(0))
		return 1;
	if (Test(1))
		return 1;

	printf("Done successfully!\n");
	return 0;
}
