#include <exceptions/exceptions.h>
#include <eventHandling/eventHandlers.h>
#include <drivers/videoDriver.h>
#include <processState.h>


extern uint64_t rax_backup, rbx_backup, rcx_backup, rdx_backup, rsi_backup, rdi_backup, rbp_backup, r8_backup, r9_backup, r10_backup, r11_backup, r12_backup, r13_backup, r14_backup, r15_backup;
extern uint64_t cri_rip, cri_rsp, cri_rflags;



static BackupRegisters backupRegisters;

void saveRegisters() {
	backupRegisters.rax = rax_backup;
	backupRegisters.rbx = rbx_backup;
	backupRegisters.rcx = rcx_backup;
	backupRegisters.rdx = rdx_backup;
	backupRegisters.rsi = rsi_backup;
	backupRegisters.rdi = rdi_backup;
	backupRegisters.rbp = rbp_backup;
	backupRegisters.r8 = r8_backup;
	backupRegisters.r9 = r9_backup;
	backupRegisters.r10 = r10_backup;
	backupRegisters.r11 = r11_backup;
	backupRegisters.r12 = r12_backup;
	backupRegisters.r13 = r13_backup;
	backupRegisters.r14 = r14_backup;
	backupRegisters.r15 = r15_backup;
	backupRegisters.cri_rip = cri_rip;
	backupRegisters.cri_rsp = cri_rsp;
	backupRegisters.cri_rflags = cri_rflags;
}


static void zero_division();
static void invalid_opcode();

void exceptionDispatcher(int exception) {
	saveRegisters();
	// switch (exception) {
	// 	case ZERO_EXCEPTION_ID:
	// 		zero_division();
	// 		break;
	// 	case INVALID_OPCODE_EXCEPTION_ID:
	// 		invalid_opcode();
	// 		break;
	// }
	callExceptionHandler(exception, &backupRegisters);
	quitProgram();
}

static void zero_division() {
	// Handler para manejar excepcíon
}

static void invalid_opcode() {
	// Handler para manejar excepcíon
}