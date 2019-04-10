// CS 654 Mini Project 01
// Problem 1 - part 2
// Group of 3 
// Ritesh patel (rrp8)
// Ketaki Kakade (kk524)
// Yash Shah (yss22)

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

public class Cracker {
	
	public static void main(String[] args) throws IOException {
		System.out.println(System.getProperty("user.dir"));
		BufferedReader br1 = new BufferedReader(new FileReader("shadow.txt"));
		MD5Shadow MD5 = new MD5Shadow();
		try {
			String line, word;
			int m,q;
			while ((line =br1.readLine()) != null) {
				//System.out.println("=================================================");
				char[] line_char = line.toCharArray();
				
				char[] user = new char[60];
				char[] salt = new char[60];
				char[] g_hash = new char[60];
				
				for (int j = 0; j < line.length(); j++) {
					if ((line.charAt(j) == ':') == true) {
						for (int k = 0; k < j; k++) {
							user[k] = line_char[k];
						}
						for (int k = j + 4; k < line.length(); k++) {
							if (line.charAt(k) == '$') {
								int c=0;
								for (q = 0, m = j + 4; m <= k - 1; q++, m++, j++) {
									salt[q] = line_char[m];
									c=m;
								}
								c++;
								if(line.charAt(c) == '$') {
									int b=c + 1;
									for (;c <= line.length(); c++ ) {
										if(line.charAt(c) == ':') {
											for(int a = 0;b <=c - 1; a++, b++) {
												g_hash[a] = line_char[b];
											}
//											System.out.println("Reading the Following details: ");
//											System.out.println(user);
//											System.out.println(salt);
//											System.out.println(g_hash);
											
											BufferedReader br2 = new BufferedReader(new FileReader("common-passwords.txt"));
											//int i=1;
											while((word = br2.readLine()) != null) {
												//System.out.println("checking Word: "+word);
												
												String salt_in = String.copyValueOf(salt).trim();
												String g_hash_in = String.copyValueOf(g_hash).trim();
												String user_in = String.copyValueOf(user).trim();
												
												String r1;
												r1 = MD5.crypt(word, salt_in);
												 //System.out.println(user_in);
												 //System.out.println("Values of G_hash: "+g_hash_in);

												//System.out.println("Values of G_hash: "+g_hash_in);
												 if(r1.equalsIgnoreCase(g_hash_in)){
													 //System.out.println(i);	i++;
													//System.out.println("MD5 value is: " +r1);
													 System.out.println(user_in+":"+word);
													 //continue;
												 }
											}
											break;
										}
									}
								}
							}
						}
					break;}
				}
			}
		}
				catch (Exception e) {
			e.printStackTrace();
		}finally {
			br1.close();
		}
	}

}
