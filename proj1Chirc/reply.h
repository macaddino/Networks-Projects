/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Reply codes and functions
 *
 */


#ifndef REPLY_H_
#define REPLY_H_

#include "structures.h"

#define RPL_WELCOME 	"001"
#define RPL_YOURHOST 	"002"
#define RPL_CREATED		"003"
#define RPL_MYINFO		"004"

#define RPL_LUSERCLIENT		"251"
#define RPL_LUSEROP			"252"
#define RPL_LUSERUNKNOWN	"253"
#define RPL_LUSERCHANNELS	"254"
#define RPL_LUSERME			"255"

#define RPL_AWAY			"301"
#define RPL_UNAWAY          "305"
#define RPL_NOWAWAY         "306"

#define RPL_WHOISUSER		"311"
#define RPL_WHOISSERVER		"312"
#define RPL_WHOISOPERATOR		"313"
#define RPL_WHOISIDLE		"317"
#define RPL_ENDOFWHOIS		"318"
#define RPL_WHOISCHANNELS		"319"

#define RPL_WHOREPLY		"352"
#define RPL_ENDOFWHO		"315"

#define RPL_LIST			"322"
#define RPL_LISTEND			"323"

#define RPL_CHANNELMODEIS	"324"

#define RPL_NOTOPIC			"331"
#define RPL_TOPIC			"332"

#define RPL_NAMREPLY		"353"
#define RPL_ENDOFNAMES		"366"

#define RPL_MOTDSTART		"375"
#define RPL_MOTD			"372"
#define RPL_ENDOFMOTD		"376"

#define RPL_YOUREOPER		"381"

#define ERR_NOSUCHNICK			"401"
#define ERR_NOSUCHCHANNEL		"403"
#define ERR_CANNOTSENDTOCHAN	"404"
#define ERR_UNKNOWNCOMMAND		"421"
#define ERR_NOMOTD              "422"
#define ERR_NICKNAMEINUSE		"433"
#define ERR_USERNOTINCHANNEL	"441"
#define ERR_NOTONCHANNEL		"442"
#define ERR_NOTREGISTERED		"451"
#define ERR_ALREADYREGISTRED	"462"
#define ERR_PASSWDMISMATCH      "464"
#define ERR_UNKNOWNMODE			"472"
#define ERR_CHANOPRIVSNEEDED	"482"
#define ERR_UMODEUNKNOWNFLAG	"501"
#define ERR_USERSDONTMATCH		"502"

#define RPL_WELCOME_MSG		":Welcome to the Internet Relay Network %s!%s@%s"
#define RPL_YOURHOST_MSG	":Your host is %s, running version %s"
#define RPL_CREATED_MSG		":This server was created %s"
#define RPL_MYINFO_MSG		"%s %s %s %s"
#define ERR_UNKNOWNCOMMAND_MSG  "%s :Unknown command"
#define ERR_ALREADYREGISTERED_MSG ":Unauthorized command (already registered)"
#define ERR_NICKNAMEINUSE_MSG "%s :Nickname is already in use"
#define RPL_MOTDSTART_MSG	":- %s Message of the day - "
#define RPL_MOTD_MSG 		":- %s"
#define RPL_ENDOFMOTD_MSG 	":- End of MOTD command"
#define ERR_NOSUCHNICK_MSG	"%s :No such nick/channel"
#define RPL_LUSERCLIENT_MSG 	":There are %s users and %s services on %s servers"
#define RPL_LUSEROP_MSG		"%s :operator(s) online"
#define RPL_LUSERUNKNOWN_MSG	"%s :unknown connection(s)"
#define RPL_LUSERCHANNELS_MSG	"%s :channels formed"
#define RPL_LUSERME_MSG		":I have %s clients and %s servers"
#define ERR_NOMOTD_MSG ":MOTD File is missing"
#define RPL_WHOISUSER_MSG "%s %s %s * :%s"
#define RPL_WHOISSERVER_MSG "%s %s :%s"
#define RPL_ENDOFWHOIS_MSG "%s :End of WHOIS list"
#define RPL_NAMREPLY_MSG "%s %s :%s"
#define RPL_ENDOFNAMES_MSG "%s :End of NAMES list"
#define ERR_NOSUCHCHANNEL_MSG "#%s :No such channel"
#define ERR_NOTONCHANNEL_MSG "#%s :You're not on that channel"
#define ERR_CANNOTSENDTOCHAN_MSG "#%s :Cannot send to channel"
#define RPL_TOPIC_MSG "#%s :%s"
#define RPL_NOTOPIC_MSG "#%s :No topic is set"
#define RPL_LIST_MSG "%s %s :%s"
#define RPL_LISTEND_MSG ":End of LIST"
#define RPL_WHOISCHANNELS_MSG "%s :%s"
#define ERR_UMODEUNKNOWNFLAG_MSG ":Unknown MODE flag"
#define ERR_USERSDONTMATCH_MSG ":Cannot change mode for other users"
#define ERR_UNKNOWNMODE_MSG "%s :is unknown mode char to me for #%s"
#define RPL_CHANNELMODEIS_MSG "#%s +%s"
#define ERR_CHANOPRIVSNEEDED_MSG "#%s :You're not channel operator"
#define ERR_USERNOTINCHANNEL_MSG "%s #%s :They aren't on that channel"
#define ERR_CANNOTSENDTOCHAN_MSG "#%s :Cannot send to channel"
#define ERR_PASSWDMISMATCH_MSG ":Password incorrect"
#define RPL_YOUREOPER_MSG ":You are now an IRC operator"
#define RPL_UNAWAY_MSG ":You are no longer marked as being away"
#define RPL_NOWAWAY_MSG ":You have been marked as being away"
#define RPL_AWAY_MSG "%s %s"
#define RPL_WHOREPLY_MSG "%s"
#define RPL_ENDOFWHO_MSG "%s :End of WHO list"
#define RPL_WHOISOPERATOR_MSG "%s :is an IRC operator"

int send_response(int clientSocket, replyPackage * reply);

#endif /* REPLY_H_ */
