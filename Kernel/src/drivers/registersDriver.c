#include <drivers/registersDriver.h>

extern uint64_t rax_backup, rbx_backup, rcx_backup, rdx_backup, rsi_backup, rdi_backup, rsp_backup, rbp_backup, r8_backup, r9_backup, r10_backup, r11_backup, r12_backup, r13_backup, r14_backup, r15_backup;
extern uint64_t cri_rip, cri_rsp, cri_rflags;


static BackupRegisters backupRegisters;

void saveRegisters() {
	// log_to_serial("E:       ---------> Saving registers...\n");
	backupRegisters.registers.rax = rax_backup;
	backupRegisters.registers.rbx = rbx_backup;
	backupRegisters.registers.rcx = rcx_backup;
	backupRegisters.registers.rdx = rdx_backup;
	backupRegisters.registers.rsi = rsi_backup;
	backupRegisters.registers.rdi = rdi_backup;
	backupRegisters.registers.rsp = rsp_backup;
	backupRegisters.registers.rbp = rbp_backup;
	backupRegisters.registers.r8 = r8_backup;
	backupRegisters.registers.r9 = r9_backup;
	backupRegisters.registers.r10 = r10_backup;
	backupRegisters.registers.r11 = r11_backup;
	backupRegisters.registers.r12 = r12_backup;
	backupRegisters.registers.r13 = r13_backup;
	backupRegisters.registers.r14 = r14_backup;
	backupRegisters.registers.r15 = r15_backup;
	backupRegisters.cri_rip = cri_rip;
	backupRegisters.cri_rsp = cri_rsp;
	backupRegisters.cri_rflags = cri_rflags;
	// log_to_serial("E:       ---------> Guardado registers...\n");
}

BackupRegisters * getBackupRegisters() {
    return &backupRegisters;
}
