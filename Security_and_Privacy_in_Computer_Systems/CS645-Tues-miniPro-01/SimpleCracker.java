// CS 654 Mini Project 01
// Problem 1 - part 2
// Group of 3 
// Ritesh patel (rrp8)
// Ketaki Kakade (kk524)
// Yash Shah (yss22)

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class SimpleCracker {
	
	public static void main(String[] args) throws IOException {
		System.out.println(System.getProperty("user.dir"));
		BufferedReader br1 = new BufferedReader(new FileReader("shadow-simple.txt"));
		try {
			String line, word;
			while ((line =br1.readLine()) != null){
				//System.out.println("=================================================");
				//System.out.println(line);
				char[] line_char = line.toCharArray();
				//System.out.println(line_char);

				char[] user = new char[16];
				char[] salt = new char[16];
				char[] g_hash = new char[48];
				
				for (int j = 0; j < line.length(); j++) {
					if ((line.charAt(j) == ':') == true) {
						for (int k = 0; k <= j - 1; k++) {
							user[k] = line_char[k];
						}
						for (int k = j + 1; k < line.length(); k++) {
							if (line.charAt(k) == ':') {
								for (int q = 0, m = j + 1; m <= k - 1; q++, m++, j++) {
									salt[q] = line_char[j + 1];
									if (line.charAt(m + 1) == ':') {
										for (int p = 0, n = m + 2; n < line.length(); p++, n++, m++) {
											g_hash[p] = line_char[m + 2];
										}
//										System.out.println("Reading the Following details: ");
//										System.out.println(user);
//										System.out.println(salt);
//										System.out.println(g_hash);
										
										BufferedReader br2 = new BufferedReader(new FileReader("common-passwords.txt"));

										while((word = br2.readLine()) != null) {
											//System.out.println("checking Word: "+word);
											
											String salt_in = String.copyValueOf(salt).trim();
											String g_hash_in = String.copyValueOf(g_hash).trim();
											String user_in = String.copyValueOf(user).trim();
											String md_frnt = salt_in.concat(word);
											String md_back = word.concat(salt_in);
											//System.out.println(md_frnt);
											MessageDigest msgF = null, msgB = null;
											try {
												msgF = MessageDigest.getInstance("MD5");
												msgB = MessageDigest.getInstance("MD5");
												msgF.update(md_frnt.getBytes());
												msgB.update(md_frnt.getBytes());
												byte[] MD5F = msgF.digest();
												byte[] MD5B = msgB.digest();
									            StringBuffer stBufferB = new StringBuffer();
									            StringBuffer stBufferF = new StringBuffer();
									            for (byte bytes : MD5F) {
									            	stBufferF.append(String.format("%02x", bytes & 0xff));
									            }
									            for (byte bytes : MD5B) {
									        		
									            	stBufferB.append(String.format("%02x", bytes & 0xff));
									            }
									            String passF = stBufferF.toString();
									            String passB = stBufferB.toString();
									            if(passF.equalsIgnoreCase(g_hash_in) || passB.equalsIgnoreCase(g_hash_in)) {
									            	System.out.println(user_in+":"+word);
									            }
												
											} catch (NoSuchAlgorithmException e) {
												e.printStackTrace();
											}finally {
												msgF.reset();
												msgB.reset();
											}
											
										}
										
										break;
									}
								}
								break;
							}
						}
						break;
					}
				}
			}
		} catch (Exception e) {
				e.printStackTrace();
		}finally {
			br1.close();
			//br2.close();
		}
	}
}
