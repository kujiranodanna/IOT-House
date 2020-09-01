#!/usr/bin/ruby -Ku
# pepogmail_send.cgi ; Attach the image file and send mail, use the ruby gmail
# licence GPLv3 ; this scripts designed by Yamauchi Isamu 2011.11.28 update 2013.9.8
def error_cgi
        print "Content-Type:text/html;charset=UTF-8\n\n"
        print "*** CGI Error List ***<br />"
        print "#{CGI.escapeHTML($!.inspect)}<br />"
        $@.each {|x| print CGI.escapeHTML(x), "<br />"}
end

begin
  require 'cgi'
  require 'gmail'
  cgi = CGI.new()
  cgi_mail_from = cgi['mail_from']
  cgi_password = cgi['password']
  cgi_mail_to = cgi['mail_to']
  cgi_subject = cgi['subject']
  cgi_message = cgi['msg']
  cgi_server = cgi['server']
  cgi_file_name = cgi['image_file']
  cgi_file_dir = '/www/remote-hand/tmp/'
  user = 'your_account@gmail.com'
  user_password = 'your_passwrod'
  s_password = 'this_system_password'
  print "Content-Type:text/html;charset=UTF-8\n\n"
  print '<BODY BGCOLOR="#E0FFFF">'
  print "<HEAD>"
  print "<TITLE>Mail Send Process</TITLE>"
  print "</HEAD>"
  print "<BODY>"
  if cgi_password == s_password then
    gmail = Gmail.new(user,user_password)
    email = gmail.generate_message do
      from cgi_mail_from
      to cgi_mail_to
      subject cgi_subject
      body cgi_message
      if cgi_file_name != ""
        add_file cgi_file_dir + cgi_file_name
      end
    end
    gmail.deliver(email)
    gmail.logout
    print "Send gmail -Successful completion<BR>"
  elsif cgi_password != s_password then
    print "Wrong password -Abend<BR>"
  end
  print "</BODY>"
  print "</HTML>"
rescue
    error_cgi
end  
