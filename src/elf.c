/*
 * shecc - Self-Hosting and Educational C Compiler.
 *
 * shecc is freely redistributable under the BSD 2 clause license. See the
 * file "LICENSE" for information on usage and redistribution of this file.
 */

/* ELF file manipulation */

int elf_symbol_index;
int elf_symtab_index;
int elf_strtab_index;
int elf_section_index;

void elf_write_section_str(char *vals, int len) {
    for (int i = 0; i < len; i++)
        elf_section[elf_section_index++] = vals[i];
}

void elf_write_data_str(char *vals, int len) {
    for (int i = 0; i < len; i++)
        elf_data[elf_data_idx++] = vals[i];
}

void elf_write_header_byte(int val) {
    elf_header[elf_header_idx++] = val;
}

void elf_write_section_byte(char val) {
    elf_section[elf_section_index++] = val;
}

char e_extract_byte(int v, int b) {
    return (v >> (b * 8)) & 0xFF;
}

int elf_write_int(char *buf, int index, int val) {
    for (int i = 0; i < 4; i++)
        buf[index++] = e_extract_byte(val, i);
    return index;
}

void elf_write_header_int(int val) {
    elf_header_idx = elf_write_int(elf_header, elf_header_idx, val);
}

void elf_write_section_int(int val) {
    elf_section_index = elf_write_int(elf_section, elf_section_index, val);
}

void elf_write_symbol_int(int val) {
    elf_symtab_index = elf_write_int(elf_symtab, elf_symtab_index, val);
}

void elf_write_code_int(int val) {
    elf_code_idx = elf_write_int(elf_code, elf_code_idx, val);
}

void elf_generate_header(int dynamic_linking_enabled) {
    /* ELF header */
    elf_write_header_int(0x464c457f); /* Magic: 0x7F followed by ELF */
    elf_write_header_byte(1);         /* 32-bit */
    elf_write_header_byte(1);         /* little-endian */
    elf_write_header_byte(1);         /* EI_VERSION */
    elf_write_header_byte(0);         /* System V */
    elf_write_header_int(0);          /* EI_ABIVERSION */
    elf_write_header_int(0);          /* EI_PAD: unused */
    elf_write_header_int(dynamic_linking_enabled ? 3 : 2); /* ET_DYN  or ET_EXEC*/
    elf_write_header_byte(0);
    elf_write_header_byte(ELF_MACHINE);
    elf_write_header_byte(0);
    elf_write_header_int(1);                          /* ELF version */
    elf_write_header_int(ELF_START + elf_header_len); /* entry point */
    elf_write_header_int(0x34);                       /* program header offset */
    elf_write_header_int(elf_header_len + elf_code_idx + elf_data_idx + 39 +
                         elf_symtab_index +
                         elf_strtab_index); /* section header offset */
    /* flags */
    elf_write_header_int(ELF_FLAGS);
    elf_write_header_byte(0x34); /* header size */
    elf_write_header_byte(0);
    elf_write_header_byte(0x20); /* program header size */
    elf_write_header_byte(0);
    elf_write_header_byte(1); /* number of program headers */
    elf_write_header_byte(0);
    elf_write_header_byte(0x28); /* section header size */
    elf_write_header_byte(0);
    elf_write_header_byte(6); /* number of sections */
    elf_write_header_byte(0);
    elf_write_header_byte(5); /* section index with names */
    elf_write_header_byte(0);

    if (dynamic_linking_enabled) {
        /* Add program header - PT_INTERP */
        elf_write_header_int(3);                          /* PT_INTERP */
        elf_write_header_int(elf_header_len);             /* offset of segment */
        elf_write_header_int(ELF_START + elf_header_len); /* virtual address */
        elf_write_header_int(ELF_START + elf_header_len); /* physical address */
        elf_write_header_int(20);                         /* size in file */
        elf_write_header_int(20);                         /* size in memory */
        elf_write_header_int(4);                          /* flags */
        elf_write_header_int(4);                          /* alignment */
    }

    /* program header - code and data combined */
    elf_write_header_int(1);                           /* PT_LOAD */
    elf_write_header_int(elf_header_len);              /* offset of segment */
    elf_write_header_int(ELF_START + elf_header_len);  /* virtual address */
    elf_write_header_int(ELF_START + elf_header_len);  /* physical address */
    elf_write_header_int(elf_code_idx + elf_data_idx); /* size in file */
    elf_write_header_int(elf_code_idx + elf_data_idx); /* size in memory */
    elf_write_header_int(7);                           /* flags */
    elf_write_header_int(4);                           /* alignment */
}

void elf_generate_dynamic_sections(){
    /* .dynamic section*/
    elf_write_section_int(1); /* DT_NEEDED */
    elf_write_section_int(elf_strtab_index); /* offset in .dynstr */
    elf_write_section_int(0); /* DT_NULL */
    elf_write_section_int(0); /* End of .dynamic */

    /* .interp section */
    elf_write_section_str("/lib/ld-linux.so.3", 18); /* interpreter */
    elf_write_section_int(0); /* End of .interp */

    /* .dynstr section */
    elf_write_section_str("libc.so.6", 9); /* dynamic linked library */
    elf_write_section_byte(0); /* End of .dynsym */

    /* dynsym section*/
    elf_write_section_int(0); /* NULL entry */
    elf_write_section_int(0); 
    elf_write_section_int(0);
    elf_write_section_byte(0);
    elf_write_section_byte(0);
    elf_write_section_byte(0 & 0xFF);
    elf_write_section_byte(0 >> 8 & 0xFF); /* SHN_UNDEF*/

    elf_write_section_int(1); /* offset to "libc.so.6" */
    elf_write_section_int(0); 
    elf_write_section_int(0);
    elf_write_section_byte(0x10); /* STB_GLOBAL | STT_OBJECT */
    elf_write_section_byte(0);
    elf_write_section_byte(0 & 0xFF);
    elf_write_section_byte(0 >> 8 & 0xFF); /* SHN_UNDEF */

    elf_write_section_int(0); /* End of .dynsym*/

    /* .rela.plt section */
    elf_write_section_int(0);
    elf_write_section_int(0x16); /* R_ARM_JUMP_SLOT */
    elf_write_section_int(0); /* r_addend*/

    /* .plt section */
    // ARM32 PLT entry
    elf_write_code_int(0xe28fc600);  // add ip, pc, #0
    elf_write_code_int(0xe28cca00);  // add ip, ip, #0
    elf_write_code_int(0xe5bcf000);  // ldr pc, [ip, #0]!

    // .got section
    elf_write_section_int(0);  // GOT[0]: Reserved
    elf_write_section_int(0);  // GOT[1]: Reserved
    elf_write_section_int(0);  // GOT[2]: "libc.so.6" entry
}

void elf_generate_sections(int dynamic_linking_enabled) {
    if (dynamic_linking_enabled) {
        elf_generate_dynamic_sections();
    }

    /* symtab section */
    for (int b = 0; b < elf_symtab_index; b++)
        elf_write_section_byte(elf_symtab[b]);

    /* strtab section */
    for (int b = 0; b < elf_strtab_index; b++)
        elf_write_section_byte(elf_strtab[b]);

    /* shstr section; len = 39(static), 93(dynamic) */
    elf_write_section_byte(0);
    elf_write_section_str(".shstrtab", 9);
    elf_write_section_byte(0);
    elf_write_section_str(".text", 5);
    elf_write_section_byte(0);
    elf_write_section_str(".data", 5);
    elf_write_section_byte(0);
    if (dynamic_linking_enabled) {
        elf_write_section_str(".dynstr", 7);
        elf_write_section_byte(0);
        elf_write_section_str(".dynsym", 7);
        elf_write_section_byte(0);
        elf_write_section_str(".dynamic", 8);
        elf_write_section_byte(0);
        elf_write_section_str(".interp", 8);
        elf_write_section_byte(0);
        elf_write_section_str(".rela.plt", 9);
        elf_write_section_byte(0);
        elf_write_section_str(".plt", 4);
        elf_write_section_byte(0);
        elf_write_section_str(".got", 4);
        elf_write_section_byte(0);
    }
    elf_write_section_str(".symtab", 7);
    elf_write_section_byte(0);
    elf_write_section_str(".strtab", 7);
    elf_write_section_byte(0);

    /* section header table */

    /* NULL section */
    elf_write_section_int(0);
    elf_write_section_int(0);
    elf_write_section_int(0);
    elf_write_section_int(0);
    elf_write_section_int(0);
    elf_write_section_int(0);
    elf_write_section_int(0);
    elf_write_section_int(0);
    elf_write_section_int(0);
    elf_write_section_int(0);

    if (dynamic_linking_enabled) {
        // .interp section header
        elf_write_section_int(48);                          
        elf_write_section_int(1);                          
        elf_write_section_int(2);                           
        elf_write_section_int(ELF_START + elf_header_len);  
        elf_write_section_int(elf_header_len);              
        elf_write_section_int(19);                          
        elf_write_section_int(0);                           
        elf_write_section_int(0);                           
        elf_write_section_int(1);                           
        elf_write_section_int(0);                           

        // .dynsym section header
        elf_write_section_int(31);                               
        elf_write_section_int(11);                               
        elf_write_section_int(2);                                
        elf_write_section_int(ELF_START + elf_header_len + 19);  
        elf_write_section_int(elf_header_len + 19);              
        elf_write_section_int(33);                               
        elf_write_section_int(4);                                
        elf_write_section_int(1);                                
        elf_write_section_int(4);                                
        elf_write_section_int(16);                              

        // .dynstr section header
        elf_write_section_int(23);                                    
        elf_write_section_int(3);                                     
        elf_write_section_int(2);                                     
        elf_write_section_int(ELF_START + elf_header_len + 19 + 33); 
        elf_write_section_int(elf_header_len + 19 + 33);             
        elf_write_section_int(10);                                    
        elf_write_section_int(0);                                     
        elf_write_section_int(0);                                    
        elf_write_section_int(1);                                    
        elf_write_section_int(0);                                     

        // .dynamic section header
        elf_write_section_int(39);                              
        elf_write_section_int(6);                                
        elf_write_section_int(3);                                
        elf_write_section_int(ELF_START + elf_header_len + 62);  
        elf_write_section_int(elf_header_len + 62);             
        elf_write_section_int(16);                               
        elf_write_section_int(4);                                
        elf_write_section_int(0);                                
        elf_write_section_int(4);                                
        elf_write_section_int(8);                               

        // .rela.plt section header
        elf_write_section_int(57);                               
        elf_write_section_int(4);                                
        elf_write_section_int(66);                               
        elf_write_section_int(ELF_START + elf_header_len + 78);  
        elf_write_section_int(elf_header_len + 78);              
        elf_write_section_int(12);                               
        elf_write_section_int(2);                               
        elf_write_section_int(8);                                
        elf_write_section_int(4);                                
        elf_write_section_int(12);                              

        // .plt section header
        elf_write_section_int(67);                               
        elf_write_section_int(1);                                
        elf_write_section_int(6);                                
        elf_write_section_int(ELF_START + elf_header_len + 90);  
        elf_write_section_int(elf_header_len + 90);              
        elf_write_section_int(12);                               
        elf_write_section_int(0);                                
        elf_write_section_int(0);                                
        elf_write_section_int(4);                                
        elf_write_section_int(16);                               

        // .got section header
        elf_write_section_int(72);                              
        elf_write_section_int(1);                                
        elf_write_section_int(3);                                
        elf_write_section_int(ELF_START + elf_header_len + 102);  
        elf_write_section_int(elf_header_len + 102);             
        elf_write_section_int(12);                               
        elf_write_section_int(0);                                
        elf_write_section_int(0);                                
        elf_write_section_int(4);                                
        elf_write_section_int(4);                                
    }



    /* .text */
    elf_write_section_int(0xb);
    elf_write_section_int(1);
    elf_write_section_int(7);
    elf_write_section_int(ELF_START + dynamic_linking_enabled ? elf_header_len + 114 : elf_header_len);
    elf_write_section_int(elf_header_len);
    elf_write_section_int(elf_code_idx);
    elf_write_section_int(0);
    elf_write_section_int(0);
    elf_write_section_int(4);
    elf_write_section_int(0);

    /* .data */
    elf_write_section_int(0x11);
    elf_write_section_int(1);
    elf_write_section_int(3);
    elf_write_section_int(elf_code_start + ((dynamic_linking_enabled ? 114 : 0) + elf_header_len) + elf_code_idx);
    elf_write_section_int(elf_header_len + elf_code_idx);
    elf_write_section_int(elf_data_idx);
    elf_write_section_int(0);
    elf_write_section_int(0);
    elf_write_section_int(4);
    elf_write_section_int(0);

    /* .symtab */
    elf_write_section_int(0x17);
    elf_write_section_int(2);
    elf_write_section_int(0);
    elf_write_section_int(0);
    elf_write_section_int((dynamic_linking_enabled ? 114 : 0) + elf_header_len + elf_code_idx + elf_data_idx);
    elf_write_section_int(elf_symtab_index); /* size */
    elf_write_section_int(4);
    elf_write_section_int(elf_symbol_index);
    elf_write_section_int(4);
    elf_write_section_int(16);

    /* .strtab */
    elf_write_section_int(0x1f);
    elf_write_section_int(3);
    elf_write_section_int(0);
    elf_write_section_int(0);
    elf_write_section_int((dynamic_linking_enabled ? 114 : 0) + elf_header_len + elf_code_idx + elf_data_idx +
                          elf_symtab_index);
    elf_write_section_int(elf_strtab_index); /* size */
    elf_write_section_int(0);
    elf_write_section_int(0);
    elf_write_section_int(1);
    elf_write_section_int(0);

    /* .shstr */
    elf_write_section_int(1);
    elf_write_section_int(3);
    elf_write_section_int(0);
    elf_write_section_int(0);
    elf_write_section_int(elf_header_len + elf_code_idx + elf_data_idx +
                          elf_symtab_index + elf_strtab_index);
    elf_write_section_int(dynamic_linking_enabled ? 93 : 39); 
    elf_write_section_int(0);
    elf_write_section_int(0);
    elf_write_section_int(1);
    elf_write_section_int(0);
}

void elf_align() {
    int remainder = elf_data_idx & 3;
    if (remainder)
        elf_data_idx += (4 - remainder);

    remainder = elf_symtab_index & 3;
    if (remainder)
        elf_symtab_index += (4 - remainder);

    remainder = elf_strtab_index & 3;
    if (remainder)
        elf_strtab_index += (4 - remainder);
}

void elf_add_symbol(char *symbol, int len, int pc) {
    elf_write_symbol_int(elf_strtab_index);
    elf_write_symbol_int(pc);
    elf_write_symbol_int(0);
    elf_write_symbol_int(pc == 0 ? 0 : 1 << 16);

    strncpy(elf_strtab + elf_strtab_index, symbol, len);
    elf_strtab_index += len;
    elf_strtab[elf_strtab_index++] = 0;
    elf_symbol_index++;
}

void elf_generate(int dynamic_linking_enabled, char *outfile) {
    elf_symbol_index = 0;
    elf_symtab_index = 0;
    elf_strtab_index = 0;
    elf_section_index = 0;

    elf_align();
    elf_generate_header(dynamic_linking_enabled);
    elf_generate_sections(dynamic_linking_enabled);

    if (!outfile)
        outfile = "a.out";

    FILE *fp = fopen(outfile, "wb");
    for (int i = 0; i < elf_header_idx; i++)
        fputc(elf_header[i], fp);
    for (int i = 0; i < elf_code_idx; i++)
        fputc(elf_code[i], fp);
    for (int i = 0; i < elf_data_idx; i++)
        fputc(elf_data[i], fp);
    for (int i = 0; i < elf_section_index; i++)
        fputc(elf_section[i], fp);
    fclose(fp);
}
