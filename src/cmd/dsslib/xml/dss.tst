# regression tests for the dss xml method

TITLE + xml

export TZ=EST5EDT LC_ALL=C.UTF-8

VIEW data

TEST 01 'XML data'

	EXEC -I$data -x xml '(user.followers_count>=10000)|{print "%(user.name)s|%(user.followers_count)s"}' followers.xml
		OUTPUT - $'A Fun Zone|11174
Michael Saxon|18786
Jokes Tweet|11354
The Twit Cleaner|17350
UK Travel|28653'

	EXEC -I$data -x xml '(user.followers_count<10||user.followers_count>10000)|{print "%(user.name)s|%(user.followers_count)s"}' followers.xml
		OUTPUT - $'Harikrishnan|5
A Fun Zone|11174
Crystal Haynes|8
louis nduru|7
Michael Saxon|18786
Jokes Tweet|11354
Azzumar|2
The Twit Cleaner|17350
UK Travel|28653'

	EXEC -I$data -x xml '(user.status.id==11951251702||user.status.id==11944865950||user.status.id==11799602058)' followers.xml
		OUTPUT - $'<user>
  <id>35242572</id>
  <name>John Iley</name>
  <screen_name>johniley</screen_name>
  <location>Gateshead</location>
  <description>IT person, self confessed work-a-holic, wannabe Webpage/Graphic designer &amp; wannabe author</description>
  <profile_image_url>http://a3.twimg.com/profile_images/362048981/twitterProfilePhoto_normal.jpg</profile_image_url>
  <url>http://www.johniley.com</url>
  <protected>false</protected>
  <followers_count>136</followers_count>
  <profile_background_color>9AE4E8</profile_background_color>
  <profile_text_color>333333</profile_text_color>
  <profile_link_color>2100b3</profile_link_color>
  <profile_sidebar_fill_color>6d8ee8</profile_sidebar_fill_color>
  <profile_sidebar_border_color>BDDCAD</profile_sidebar_border_color>
  <friends_count>177</friends_count>
  <created_at>Sat Apr 25 16:02:04 +0000 2009</created_at>
  <favourites_count>4</favourites_count>
  <utc_offset>0</utc_offset>
  <time_zone>London</time_zone>
  <profile_background_image_url>http://a1.twimg.com/profile_background_images/54577154/twitterback.jpg</profile_background_image_url>
  <profile_background_tile>false</profile_background_tile>
  <notifications>false</notifications>
  <geo_enabled>false</geo_enabled>
  <verified>false</verified>
  <following>false</following>
  <statuses_count>3273</statuses_count>
  <lang>en</lang>
  <contributors_enabled>false</contributors_enabled>
  <status>
    <created_at>Sat Apr 10 19:01:18 +0000 2010</created_at>
    <id>11951251702</id>
    <text>@delvestaxis I agree! She\'s a babe!</text>
    <source>web</source>
    <truncated>false</truncated>
    <in_reply_to_status_id>11948705594</in_reply_to_status_id>
    <in_reply_to_user_id>18716042</in_reply_to_user_id>
    <favorited>false</favorited>
    <in_reply_to_screen_name>delvestaxis</in_reply_to_screen_name>
    <geo/>
    <coordinates/>
    <place/>
    <contributors/>
  </status>
</user>
<user>
  <id>15433271</id>
  <name>Will Swain</name>
  <screen_name>willswain</screen_name>
  <location>Seaford, UK</location>
  <description>Web developer and father of 4</description>
  <profile_image_url>http://a1.twimg.com/profile_images/609408966/s531407117_1152693_9559_normal.jpg</profile_image_url>
  <url>http://www.hothorse.com</url>
  <protected>false</protected>
  <followers_count>435</followers_count>
  <profile_background_color>131516</profile_background_color>
  <profile_text_color>333333</profile_text_color>
  <profile_link_color>009999</profile_link_color>
  <profile_sidebar_fill_color>efefef</profile_sidebar_fill_color>
  <profile_sidebar_border_color>eeeeee</profile_sidebar_border_color>
  <friends_count>428</friends_count>
  <created_at>Mon Jul 14 22:28:43 +0000 2008</created_at>
  <favourites_count>0</favourites_count>
  <utc_offset>0</utc_offset>
  <time_zone>London</time_zone>
  <profile_background_image_url>http://s.twimg.com/a/1270575596/images/themes/theme14/bg.gif</profile_background_image_url>
  <profile_background_tile>true</profile_background_tile>
  <notifications>false</notifications>
  <geo_enabled>false</geo_enabled>
  <verified>false</verified>
  <following>false</following>
  <statuses_count>3685</statuses_count>
  <lang>en</lang>
  <contributors_enabled>false</contributors_enabled>
  <status>
    <created_at>Sat Apr 10 16:31:11 +0000 2010</created_at>
    <id>11944865950</id>
    <text>@jamesallenuk good stuff. Got my eyes on that as my next phone, so be good to hear how you find it.</text>
    <source>&lt;a href=&quot;http://www.tweetdeck.com&quot; rel=&quot;nofollow&quot;&gt;TweetDeck&lt;/a&gt;</source>
    <truncated>false</truncated>
    <in_reply_to_status_id>11942212891</in_reply_to_status_id>
    <in_reply_to_user_id>31143984</in_reply_to_user_id>
    <favorited>false</favorited>
    <in_reply_to_screen_name>jamesallenuk</in_reply_to_screen_name>
    <geo/>
    <coordinates/>
    <place/>
    <contributors/>
  </status>
</user>
<user>
  <id>93576843</id>
  <name>ManhattanProject</name>
  <screen_name>MProjectUT</screen_name>
  <location>Salt Lake City</location>
  <description></description>
  <profile_image_url>http://a1.twimg.com/profile_images/550863136/loz_normal.jpg</profile_image_url>
  <url>http://Myspace.com/manhattanprojectut</url>
  <protected>false</protected>
  <followers_count>111</followers_count>
  <profile_background_color>000000</profile_background_color>
  <profile_text_color>666666</profile_text_color>
  <profile_link_color>2FC2EF</profile_link_color>
  <profile_sidebar_fill_color>252429</profile_sidebar_fill_color>
  <profile_sidebar_border_color>181A1E</profile_sidebar_border_color>
  <friends_count>408</friends_count>
  <created_at>Mon Nov 30 06:36:27 +0000 2009</created_at>
  <favourites_count>0</favourites_count>
  <utc_offset>-25200</utc_offset>
  <time_zone>Mountain Time (US &amp; Canada)</time_zone>
  <profile_background_image_url>http://a3.twimg.com/profile_background_images/56841913/header.jpg</profile_background_image_url>
  <profile_background_tile>false</profile_background_tile>
  <notifications>false</notifications>
  <geo_enabled>false</geo_enabled>
  <verified>false</verified>
  <following>false</following>
  <statuses_count>49</statuses_count>
  <lang>en</lang>
  <contributors_enabled>false</contributors_enabled>
  <status>
    <created_at>Thu Apr 08 01:58:45 +0000 2010</created_at>
    <id>11799602058</id>
    <text>Get Amethyst to 20,000 plays and a new song goes up!</text>
    <source>&lt;a href=&quot;/devices&quot; rel=&quot;nofollow&quot;&gt;txt&lt;/a&gt;</source>
    <truncated>false</truncated>
    <in_reply_to_status_id></in_reply_to_status_id>
    <in_reply_to_user_id></in_reply_to_user_id>
    <favorited>false</favorited>
    <in_reply_to_screen_name></in_reply_to_screen_name>
    <geo/>
    <coordinates/>
    <place/>
    <contributors/>
  </status>
</user>'

TEST 02 'JSON data'

	EXEC -I$data -x xml '{print "%(user.screen_name)s|%(user.id)s|%(job)s"}' basics.json
		OUTPUT - $'bozo|123|clown\ndewey|456|sidekick\ngern|789|painter'

	EXEC -I$data -x xml '(user.id!="456")|{print "%(user.screen_name)s|%(user.id)s|%(job)s"}' basics.json
		OUTPUT - $'bozo|123|clown\ngern|789|painter'

	EXEC -I$data -x xml '(user.id!=456)|{print "%(user.screen_name)s|%(user.id)s|%(job)s"}' basics.json

	EXEC -I$data -x xml '(user.id<400||user.id>500)|{print "%(user.screen_name)s|%(user.id)s|%(job)s"}' basics.json

	EXEC -I$data -x xml '(delete!=0)|{count}' test.json
		OUTPUT - $'5/100'

	EXEC -I$data -x xml '(user==0)|{count}' test.json

	EXEC -I$data -x xml '(delete==0)|{count}' test.json
		OUTPUT - $'95/100'

	EXEC -I$data -x xml '(user!=0)|{count}' test.json

	EXEC -I$data -x xml '(delete!=0)' test.json
		OUTPUT - $'{"delete":{"status":{"id":12189009100,"user_id":132553242}}}
{"delete":{"status":{"id":12467858100,"user_id":68741233}}}
{"delete":{"status":{"id":12467859700,"user_id":110919295}}}
{"delete":{"status":{"id":12464861801,"user_id":132852941}}}
{"delete":{"status":{"id":12466974802,"user_id":96445255}}}'

	EXEC -I$data -x xml '(id==12467874502||id==12467875701)' test.json
		OUTPUT - $'{"text":"For me, dere r only 5 stars in Indian team-Sachin,Saurav,Dravid,Kumble,Azhar...No offense to the earlier players, but I\'ve grown seeing them","truncated":false,"created_at":"Mon Apr 19 17:52:02 +0000 2010","in_reply_to_screen_name":null,"source":"<a href=\\"http://www.seesmic.com/\\" rel=\\"nofollow\\">Seesmic</a>","contributors":null,"geo":null,"place":null,"in_reply_to_user_id":null,"in_reply_to_status_id":null,"user":{"favourites_count":3,"profile_background_color":"ffffff","profile_image_url":"http://a1.twimg.com/profile_images/766419408/DSC04255_normal.JPG","time_zone":"Mumbai","created_at":"Wed Sep 17 07:18:33 +0000 2008","statuses_count":3974,"profile_text_color":"222222","description":"Simple, fun loving and friendly... No complications !","screen_name":"svinz","lang":"en","profile_background_image_url":"http://a1.twimg.com/profile_background_images/50156226/mqMJtribute.br.jpg","location":"Chennai","contributors_enabled":false,"following":null,"profile_link_color":"bfbfbf","notifications":null,"profile_background_tile":false,"geo_enabled":false,"profile_sidebar_fill_color":"a80000","followers_count":260,"protected":false,"verified":false,"url":"http://vinayshivashankar.wordpress.com","name":"Vinay S","friends_count":390,"profile_sidebar_border_color":"222222","id":16325029,"utc_offset":19800},"favorited":false,"id":12467874502,"coordinates":null}
{"text":"@ryuji_gushiken \\u3046\\u308f\\u3042\\u3042\\u3042\\u3042\\u3001\\u30ad\\u30ea\\u30c3\\u3068\\u3057\\u305f\\u6817\\u5c71\\u5343\\u660e\\u3002\\u3002\\u898b\\u305f\\u304f\\u306a\\u3063\\u3066\\u304d\\u305f\\u3002\\u3002","truncated":false,"created_at":"Mon Apr 19 17:52:04 +0000 2010","in_reply_to_screen_name":"ryuji_gushiken","source":"<a href=\\"http://www.tweetdeck.com/\\" rel=\\"nofollow\\">TweetDeck</a>","contributors":null,"geo":null,"place":null,"in_reply_to_user_id":5729482,"in_reply_to_status_id":12467504152,"user":{"favourites_count":10,"profile_background_color":"9ae4e8","profile_image_url":"http://a3.twimg.com/profile_images/53694109/suneno_normal.jpeg","time_zone":"Tokyo","created_at":"Fri May 02 22:37:41 +0000 2008","statuses_count":1167,"profile_text_color":"000000","description":"31\\u6b73\\u3002\\u6642\\u3005\\u3001\\u3075\\u3089\\u3063\\u3068\\u6d77\\u5916\\u306b\\u9003\\u3052\\u307e\\u3059\\u3002\\u643a\\u5e2f\\u3001\\u30ac\\u30b8\\u30a7\\u30c3\\u30c8\\u306b\\u3081\\u3056\\u3081\\u308b\\u3002iPhone3G\\u611b\\u7528\\u4e2d\\u3002\\u305f\\u3060\\u3057\\u3001\\u60c5\\u7dd2\\u4e0d\\u5b89\\u5b9a\\u3002","screen_name":"itowwww","lang":"ja","profile_background_image_url":"http://s.twimg.com/a/1271213136/images/themes/theme1/bg.png","location":"\\u795e\\u5948\\u5ddd\\u770c","contributors_enabled":false,"following":null,"profile_link_color":"0000ff","notifications":null,"profile_background_tile":false,"geo_enabled":true,"profile_sidebar_fill_color":"e0ff92","followers_count":121,"protected":false,"verified":false,"url":null,"name":"itowwww","friends_count":207,"profile_sidebar_border_color":"87bc44","id":14632066,"utc_offset":32400},"favorited":false,"id":12467875701,"coordinates":null}'

	EXEC -I$data -x xml '(id==12467874502||id==12467875701)|{print "%(text)s"}' test.json
		OUTPUT - $'For me, dere r only 5 stars in Indian team-Sachin,Saurav,Dravid,Kumble,Azhar...No offense to the earlier players, but I\'ve grown seeing them\n@ryuji_gushiken \\u3046\\u308f\\u3042\\u3042\\u3042\\u3042\\u3001\\u30ad\\u30ea\\u30c3\\u3068\\u3057\\u305f\\u6817\\u5c71\\u5343\\u660e\\u3002\\u3002\\u898b\\u305f\\u304f\\u306a\\u3063\\u3066\\u304d\\u305f\\u3002\\u3002'
 
	EXEC -I$data -x xml '(id==12467874502||id==12467875701)|{print "%(text:expand=wide)s"}' test.json
		OUTPUT - $'For me, dere r only 5 stars in Indian team-Sachin,Saurav,Dravid,Kumble,Azhar...No offense to the earlier players, but I\'ve grown seeing them\n@ryuji_gushiken \343\201\206\343\202\217\343\201\202\343\201\202\343\201\202\343\201\202\343\200\201\343\202\255\343\203\252\343\203\203\343\201\250\343\201\227\343\201\237\346\240\227\345\261\261\345\215\203\346\230\216\343\200\202\343\200\202\350\246\213\343\201\237\343\201\217\343\201\252\343\201\243\343\201\246\343\201\215\343\201\237\343\200\202\343\200\202'

	EXEC -I$data -x xml '(id==12467874502||id==12467875701)|{print "%(:expand=wide:)s%(text)s"}' test.json

	EXEC -I$data -x xml '(user.screen_name!="")|{count}' twit.json
		OUTPUT - $'94/100'

	EXEC -I$data -x xml '(delete==0&&html::ref(source)!="web")|{print "%(html::ref(source))s"}' test.json
		OUTPUT - $'http://www.facebook.com/twitter
http://www.ning.com
http://www.tweetdeck.com/
http://movatwitter.jp/
http://lab.r246.jp/twicca/
http://www.trinketsoftware.com/Twikini
http://www.tweetdeck.com/
http://ubertwitter.com
http://www.tweetdeck.com
http://www.google.com/support/youtube/bin/answer.py?hl=en&answer=164577
http://www.tweetdeck.com
http://twitterrific.com
http://www.stone.com/Twittelator
http://movatwitter.jp/
http://www.atebits.com/
http://cotweet.com/?utm_source=sp1
http://blackberry.com/twitter
http://www.tweetdeck.com
http://m.twitter.com/
http://mobile.twitter.com
http://apiwiki.twitter.com/
http://www.seesmic.com/
http://www.ping.fm/
http://www.atebits.com/
http://apiwiki.twitter.com/
http://www.facebook.com/twitter
http://apiwiki.twitter.com/
http://www.mixero.com
http://www.tweetdeck.com
http://www.tweetdeck.com/
http://www.icq.com/download/icq/
http://dabr.co.uk
http://echofon.com/
http://echofon.com/
http://www.tweetdeck.com
http://dabr.co.uk
http://www.tweetdeck.com
http://echofon.com/
http://www.tweetdeck.com
/devices
http://echofon.com/
http://blackberry.com/twitter
http://code.google.com/p/pocketwit/
http://apiwiki.twitter.com/
http://www.sdn-project.net/labo/Nazrin_bot.php
http://www.tweetdeck.com/
http://itweet.net/
http://echofon.com/
http://www.nandemoyashop.com/
http://d.hatena.ne.jp/ashr10/'

	EXEC -I$data -x xml '(delete==0&&html::ref(source)!="web")|{print "%((time_t)created_at:%K)s|%(html::ref(source))s"}' test.json
		OUTPUT - $'2010-04-19+13:52:00|http://www.facebook.com/twitter
2010-04-19+13:52:00|http://www.ning.com
2010-04-19+13:52:00|http://www.tweetdeck.com/
2010-04-19+13:52:00|http://movatwitter.jp/
2010-04-19+13:52:00|http://lab.r246.jp/twicca/
2010-04-19+13:52:00|http://www.trinketsoftware.com/Twikini
2010-04-19+13:52:00|http://www.tweetdeck.com/
2010-04-19+13:52:01|http://ubertwitter.com
2010-04-19+13:52:01|http://www.tweetdeck.com
2010-04-19+13:52:01|http://www.google.com/support/youtube/bin/answer.py?hl=en&answer=164577
2010-04-19+13:52:01|http://www.tweetdeck.com
2010-04-19+13:52:01|http://twitterrific.com
2010-04-19+13:52:01|http://www.stone.com/Twittelator
2010-04-19+13:52:01|http://movatwitter.jp/
2010-04-19+13:52:01|http://www.atebits.com/
2010-04-19+13:52:01|http://cotweet.com/?utm_source=sp1
2010-04-19+13:52:01|http://blackberry.com/twitter
2010-04-19+13:52:01|http://www.tweetdeck.com
2010-04-19+13:52:01|http://m.twitter.com/
2010-04-19+13:52:01|http://mobile.twitter.com
2010-04-19+13:52:02|http://apiwiki.twitter.com/
2010-04-19+13:52:02|http://www.seesmic.com/
2010-04-19+13:52:02|http://www.ping.fm/
2010-04-19+13:52:02|http://www.atebits.com/
2010-04-19+13:52:02|http://apiwiki.twitter.com/
2010-04-19+13:52:02|http://www.facebook.com/twitter
2010-04-19+13:52:02|http://apiwiki.twitter.com/
2010-04-19+13:52:02|http://www.mixero.com
2010-04-19+13:52:03|http://www.tweetdeck.com
2010-04-19+13:52:03|http://www.tweetdeck.com/
2010-04-19+13:52:03|http://www.icq.com/download/icq/
2010-04-19+13:52:03|http://dabr.co.uk
2010-04-19+13:52:03|http://echofon.com/
2010-04-19+13:52:03|http://echofon.com/
2010-04-19+13:52:03|http://www.tweetdeck.com
2010-04-19+13:52:03|http://dabr.co.uk
2010-04-19+13:52:03|http://www.tweetdeck.com
2010-04-19+13:52:03|http://echofon.com/
2010-04-19+13:52:03|http://www.tweetdeck.com
2010-04-19+13:52:03|/devices
2010-04-19+13:52:04|http://echofon.com/
2010-04-19+13:52:04|http://blackberry.com/twitter
2010-04-19+13:52:04|http://code.google.com/p/pocketwit/
2010-04-19+13:52:04|http://apiwiki.twitter.com/
2010-04-19+13:52:04|http://www.sdn-project.net/labo/Nazrin_bot.php
2010-04-19+13:52:04|http://www.tweetdeck.com/
2010-04-19+13:52:04|http://itweet.net/
2010-04-19+13:52:04|http://echofon.com/
2010-04-19+13:52:04|http://www.nandemoyashop.com/
2010-04-19+13:52:04|http://d.hatena.ne.jp/ashr10/'

	EXEC -I$data -x xml '(delete==0&&html::ref(source)!="web")|{print "%((time_t)html::ref(created_at):%K)s"}' test.json
		OUTPUT - $'2010-04-19+13:52:00
2010-04-19+13:52:00
2010-04-19+13:52:00
2010-04-19+13:52:00
2010-04-19+13:52:00
2010-04-19+13:52:00
2010-04-19+13:52:00
2010-04-19+13:52:01
2010-04-19+13:52:01
2010-04-19+13:52:01
2010-04-19+13:52:01
2010-04-19+13:52:01
2010-04-19+13:52:01
2010-04-19+13:52:01
2010-04-19+13:52:01
2010-04-19+13:52:01
2010-04-19+13:52:01
2010-04-19+13:52:01
2010-04-19+13:52:01
2010-04-19+13:52:01
2010-04-19+13:52:02
2010-04-19+13:52:02
2010-04-19+13:52:02
2010-04-19+13:52:02
2010-04-19+13:52:02
2010-04-19+13:52:02
2010-04-19+13:52:02
2010-04-19+13:52:02
2010-04-19+13:52:03
2010-04-19+13:52:03
2010-04-19+13:52:03
2010-04-19+13:52:03
2010-04-19+13:52:03
2010-04-19+13:52:03
2010-04-19+13:52:03
2010-04-19+13:52:03
2010-04-19+13:52:03
2010-04-19+13:52:03
2010-04-19+13:52:03
2010-04-19+13:52:03
2010-04-19+13:52:04
2010-04-19+13:52:04
2010-04-19+13:52:04
2010-04-19+13:52:04
2010-04-19+13:52:04
2010-04-19+13:52:04
2010-04-19+13:52:04
2010-04-19+13:52:04
2010-04-19+13:52:04
2010-04-19+13:52:04'

	EXEC -I$data -x xml '{print "%(id)s"}' array.json
		OUTPUT - $'12467877002\n12467876900\n12467876803\n12467877101'

	EXEC -I$data -x xml '{print "%(id)lu|%(id)llu|%(id)Lu"}' array.json
		OUTPUT - $'12467877002|12467877002|12467877002
12467876900|12467876900|12467876900
12467876803|12467876803|12467876803
12467877101|12467877101|12467877101'

	EXEC -I$data -x xml::'<METHOD>xml</><XML><LIBRARY>time_t</><FIELD><NAME>created_at</><TYPE>time_t</></><FIELD><NAME>id</><TYPE>unsigned long</></></>' '(delete==0)|{print "%(created_at:%K)s|%(user.id)u|%(id)Lu"}' array.json                      
		OUTPUT - $'2010-04-19+13:52:06|66982026|12467877002
2010-04-19+13:52:06|123977409|12467876900
2010-04-19+13:52:06|77241389|12467876803
2010-04-19+13:52:06|37663378|12467877101'
