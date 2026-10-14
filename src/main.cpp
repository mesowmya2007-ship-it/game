#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {
struct Question {
  std::string prompt, hint;
  std::vector<std::string> answers;
  std::string answer() const { return answers.front(); }
};

class Ghost {
 public:
  virtual ~Ghost() = default;
  virtual const char* name() const = 0;
  virtual const char* greeting() const = 0;
  virtual const char* correctReply() const = 0;
  virtual const char* wrongReply() const = 0;
};
class HallGhost final : public Ghost {
 public:
  const char* name() const override { return "The Hall Ghost"; }
  const char* greeting() const override { return "A pale figure drifts down the staircase and whispers:"; }
  const char* correctReply() const override { return "The Hall Ghost bows. The answer is true."; }
  const char* wrongReply() const override { return "The Hall Ghost groans. One life slips away."; }
};
class BedroomGhost final : public Ghost {
 public:
  const char* name() const override { return "The Bedroom Ghost"; }
  const char* greeting() const override { return "From the mirror, a ghost rearranges the letters and asks:"; }
  const char* correctReply() const override { return "The Bedroom Ghost smiles through the glass."; }
  const char* wrongReply() const override { return "The Bedroom Ghost rattles the mirror. One life slips away."; }
};
class KitchenGhost final : public Ghost {
 public:
  const char* name() const override { return "The Ghost Chef"; }
  const char* greeting() const override { return "The ghost chef lifts a silver lid and asks:"; }
  const char* correctReply() const override { return "The Ghost Chef claps its hands. Deliciously correct!"; }
  const char* wrongReply() const override { return "The Ghost Chef slams the lid. One life slips away."; }
};
class LibraryGhost final : public Ghost {
 public:
  const char* name() const override { return "The Library Spirit"; }
  const char* greeting() const override { return "A whisper rises between the shelves and asks:"; }
  const char* correctReply() const override { return "The Library Spirit closes its book with a nod."; }
  const char* wrongReply() const override { return "The Library Spirit shakes its head. One life slips away."; }
};
const HallGhost hallGhost;
const BedroomGhost bedroomGhost;
const KitchenGhost kitchenGhost;
const LibraryGhost libraryGhost;
const Ghost& ghostForRoom(int room) {
  switch (room) {
    case 1: return hallGhost;
    case 2: return bedroomGhost;
    case 3: return kitchenGhost;
    default: return libraryGhost;
  }
}

class Challenge {
 public:
  virtual ~Challenge() = default;
  virtual const std::vector<Question>& pool() const = 0;
  virtual int count() const = 0;
  virtual std::string name() const = 0;
};
class RiddleChallenge final : public Challenge {
 public:
  explicit RiddleChallenge(std::vector<Question> q, std::string n) : questions_(std::move(q)), name_(std::move(n)) {}
  const std::vector<Question>& pool() const override { return questions_; }
  int count() const override { return 3; }
  std::string name() const override { return name_; }
 private:
  std::vector<Question> questions_; std::string name_;
};
class MealChallenge final : public Challenge {
 public:
  MealChallenge() : question_({
    {"The chef left a round baked base topped with tomato sauce and melted cheese, ready to cut into slices. What dish is it?", "It is round, cut into slices, and often shared with friends.", {"pizza"}},
    {"A toasted bun holds a grilled patty with lettuce and cheese. What familiar meal is it?", "It is a patty served between two halves of a bun.", {"burger", "hamburger"}},
    {"Long, thin strands are served in a bowl with sauce and vegetables. What are they called?", "You twirl or slurp these long strands; they are often served in a bowl.", {"noodles"}},
    {"A sweet baked treat is covered with frosting, and candles are set on top. What is it?", "People often bring it out for birthdays and add candles.", {"cake"}},
    {"The chef serves a cold, sweet dessert in scoops, sometimes in a cone. What is it?", "It melts if you leave it in the sun.", {"ice cream", "icecream"}},
    {"Golden strips of potato come hot from the fryer, with ketchup on the side. What are they?", "They are crispy potato sticks, often served beside a burger.", {"french fries", "fries", "chips"}},
    {"Cheese and lettuce are tucked between two slices of bread. What is this easy lunch?", "It has a filling held between slices of bread.", {"sandwich"}},
    {"A small ring-shaped sweet pastry has shiny glaze on top. What treat is it?", "It is a sweet ring-shaped pastry, also spelled 'doughnut'.", {"donut", "doughnut"}}
  }) {}
  const std::vector<Question>& pool() const override { return question_; }
  int count() const override { return 1; }
  std::string name() const override { return "Kitchen"; }
 private: std::vector<Question> question_;
};
class WordChallenge final : public Challenge {
 public:
  explicit WordChallenge(std::vector<Question> q, std::string n) : questions_(std::move(q)), name_(std::move(n)) {}
  const std::vector<Question>& pool() const override { return questions_; }
  int count() const override { return static_cast<int>(questions_.size()); }
  std::string name() const override { return name_; }
 private:
  std::vector<Question> questions_; std::string name_;
};

const RiddleChallenge hall({
  {"I have a face and two hands, but no arms. What am I?", "It hangs on a wall and tells you when the hour has come.", {"clock"}},
  {"The more you take, the more you leave behind. What are they?", "They mark the ground behind a traveler.", {"footsteps", "steps"}},
  {"I speak without a mouth and hear without ears. What am I?", "I return your words from a canyon or empty hall.", {"echo"}},
  {"What has many keys but cannot open a single lock?", "A musician may play it with both hands.", {"piano"}},
  {"What gets wetter as it dries?", "You use it after a bath.", {"towel"}},
  {"What has a neck but no head?", "It may hold water or wine.", {"bottle"}},
  {"What can travel around the world while staying in one corner?", "It is placed on an envelope.", {"stamp"}},
  {"What has cities but no houses, forests but no trees, and water but no fish?", "You can unfold it to find your way.", {"map"}}
}, "Hall Riddles");
const WordChallenge bedroom({
  {"Unscramble the letters: ROSTEF", "A large area covered with many trees.", {"forest"}},
  {"Unscramble the letters: LEDNAC", "It is made of wax and gives light when its wick is lit.", {"candle"}},
  {"Unscramble the letters: RROMIR", "You look into this to see your reflection.", {"mirror"}}
}, "Bedroom Word Scramble");
const RiddleChallenge library({
  {"How many natural moons orbit Earth?", "Look up at the night sky; our planet has one natural satellite.", {"one", "1"}},
  {"Which planet is known as the Red Planet?", "Its rusty colour can be seen even through a small telescope.", {"mars"}},
  {"What is the largest ocean on Earth?", "It lies between Asia and the Americas.", {"pacific", "pacific ocean"}},
  {"How many sides does a hexagon have?", "The prefix 'hexa' means six.", {"six", "6"}},
  {"Which gas do plants absorb from the air?", "Plants use it during photosynthesis; its formula is CO2.", {"carbon dioxide", "co2"}},
  {"What is the hardest naturally occurring substance?", "It is often cut to sparkle in jewelry.", {"diamond"}},
  {"Which is the smallest prime number?", "It is the only even prime.", {"two", "2"}},
  {"What is the closest star to Earth?", "It rises in the east each morning.", {"sun", "the sun"}}
}, "Library Questions");
const MealChallenge kitchen;

struct Game {
  int room = 0, index = 0, hallDone = 0, bedDone = 0, kitchenDone = 0, libraryDone = 0;
  int score = 0, lives = 5;
  std::time_t questionStarted = 0;
  bool pending = false, key = false, escaped = false;
  int treasureStage = 0;
  int potionUses = 2, whisperUses = 2, whisperRevealed = 0, answerKeyUses = 3;
  std::string notice, powerMessage;
  std::vector<Question> chosen[4];
};
std::unordered_map<std::string,Game> games;
std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
std::set<std::string> previousSets;

std::string lower(std::string s) { std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){return std::tolower(c);}); return s; }
std::string norm(std::string s) { s=lower(s);s.erase(std::remove_if(s.begin(),s.end(),[](unsigned char c){return !std::isalnum(c);}),s.end());return s; }
std::string decode(const std::string& s) { std::string o;for(size_t i=0;i<s.size();++i){if(s[i]=='+')o+=' ';else if(s[i]=='%'&&i+2<s.size()){o+=static_cast<char>(std::strtol(s.substr(i+1,2).c_str(),nullptr,16));i+=2;}else o+=s[i];}return o; }
std::map<std::string,std::string> fields(const std::string& b) { std::map<std::string,std::string> m;std::stringstream ss(b);std::string p;while(std::getline(ss,p,'&')){auto i=p.find('=');m[decode(p.substr(0,i))]=i==std::string::npos?"":decode(p.substr(i+1));}return m; }
std::string esc(const std::string& s){std::string o;for(char c:s){if(c=='&')o+="&amp;";else if(c=='<')o+="&lt;";else if(c=='>')o+="&gt;";else if(c=='\"')o+="&quot;";else if(c=='\'')o+="&#39;";else o+=c;}return o;}
void chooseQuestions(Game& g) {
  static const Challenge* stages[]={&hall,&bedroom,&kitchen,&library};
  std::string signature;
  for(int stage=0;stage<4;++stage){auto pool=stages[stage]->pool();std::shuffle(pool.begin(),pool.end(),rng);g.chosen[stage].assign(pool.begin(),pool.begin()+stages[stage]->count());for(const auto&q:g.chosen[stage])signature+=q.answer()+"|";}
  for(int tries=0;previousSets.count(signature)&&tries<8;++tries){signature.clear();for(int stage=0;stage<4;++stage){auto pool=stages[stage]->pool();std::shuffle(pool.begin(),pool.end(),rng);g.chosen[stage].assign(pool.begin(),pool.begin()+stages[stage]->count());for(const auto&q:g.chosen[stage])signature+=q.answer()+"|";}}
  previousSets.insert(signature);
}
int completed(const Game&g,int room){return room==1?g.hallDone:room==2?g.bedDone:room==3?g.kitchenDone:room==4?g.libraryDone:0;}
int total(int room){return room==3?1:3;}
bool stageComplete(const Game&g){return completed(g,g.room)>=total(g.room);}
constexpr int QUESTION_TIME_LIMIT = 30;
bool expireIfTimedOut(Game&g){if(g.room<1||g.room>4||g.pending||g.lives<=0||g.questionStarted==0)return false;if(std::time(nullptr)-g.questionStarted<QUESTION_TIME_LIMIT)return false;g.lives=0;g.questionStarted=0;g.notice="Sorry, you could not escape Ravenwood. The ghost caught you.";return true;}
std::string roomName(int r){switch(r){case 0:return "Ravenwood Estate";case 1:return "The Grand Hall";case 2:return "The Cursed Bedroom";case 3:return "The Haunted Kitchen";case 4:return "The Forbidden Library";case 5:return "The Secret Door";default:return "Victory";}}
std::string scene(int r){switch(r){case 1:return "hall";case 2:return "bedroom";case 3:return "kitchen";case 4:return "library";case 5:return "secret";case 6:return "victory";default:return "start";}}
std::string button(const std::string&t,const std::string&n,const std::string&v){return "<button class='button' name='"+n+"' value='"+v+"'>"+t+"</button>";}
std::string hintForm(const char* power,const char* title,int remaining){bool available=remaining>0;return "<form method='post' class='power-form'><button class='power "+std::string(available?"":"used")+"' name='power' value='"+power+"' "+(available?"":"disabled")+">"+title+" ("+std::to_string(remaining)+")</button></form>";}
std::string page(const Game&g){
  if(g.room==0){std::ifstream f("index.html");std::stringstream s;s<<f.rdbuf();return s.str();}
  if(g.room==-1)return "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>The Ghost Follows | Secrets of Ravenwood</title><link rel='stylesheet' href='/style.css'></head><body class='scene-start ghost-intro-screen'><div class='light-flicker' aria-hidden='true'></div><main class='frame'><header><a class='brand' href='/'>SECRETS <i>OF</i> RAVENWOOD</a><div class='stats'><span>♥ 5 lives</span><span>✦ 0 points</span></div></header><section class='ghost-intro'><img class='intro-ghost-art' src='/assets/ghost-encounter.png' alt='A shrouded ghost follows you through Ravenwood'><div class='ghost-intro-copy'><p class='eyebrow'>THE HUNT BEGINS</p><h1>“Escape me as soon as possible!”</h1><p>I am following you through Ravenwood. Solve each room’s riddles before I catch you.</p><form method='post' action='/'><button class='button' name='action' value='begin'>ENTER THE HALL</button></form></div></section><footer><span>THE GHOST IS CLOSE BEHIND</span><span>RUN, IF YOU CAN</span></footer></main></body></html>";
  const Question* q=nullptr;if(g.room>=1&&g.room<=4&&g.index<static_cast<int>(g.chosen[g.room-1].size()))q=&g.chosen[g.room-1][g.index];
  const bool timerActive=q&&g.room>=1&&g.room<=4&&!g.pending&&g.lives>0&&g.questionStarted>0;
  const int elapsed=timerActive?std::min(QUESTION_TIME_LIMIT,static_cast<int>(std::time(nullptr)-g.questionStarted)):0;
  const int remaining=timerActive?std::max(1,QUESTION_TIME_LIMIT-elapsed):QUESTION_TIME_LIMIT;
  std::ostringstream h;h<<"<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>Secrets of Ravenwood</title><link rel='stylesheet' href='/style.css'>";
  if(timerActive)h<<"<meta http-equiv='refresh' content='"<<remaining<<";url=/'>";
  h<<"</head><body class='scene-"<<scene(g.room)<<"'><div class='light-flicker' aria-hidden='true'></div><main class='frame'><header><a class='brand' href='/'>SECRETS <i>OF</i> RAVENWOOD</a><div class='stats'><span>♥ "<<g.lives<<" lives</span><span>✦ "<<g.score<<" points</span><span class='power-status'>Potion ("<<g.potionUses<<") · Whisper ("<<g.whisperUses<<") · Answer Keys ("<<g.answerKeyUses<<")</span></div></header>";
  if(!g.notice.empty())h<<"<div class='notice'>"<<esc(g.notice)<<"</div>";
  if(g.lives<=0&&!g.escaped){h<<"<section class='intro'><p class='eyebrow'>THE HOUSE CLAIMS ANOTHER</p><h1>Game Over</h1><p>Sorry, you could not escape Ravenwood.</p><p>Your lantern has gone dark. The ghost caught you.</p><form method='post'>"<<button("PLAY AGAIN","action","restart")<<"</form></section>";}
  else if(g.room==6||g.escaped){h<<"<section class='intro'><p class='eyebrow'>THE CURSE IS BROKEN</p><h1>Hey, congratulations!</h1><p>You have escaped Ravenwood!</p><p class='subtle'>The heart from the treasure chest granted one extra life and 500 points.</p><form method='post'>"<<button("PLAY AGAIN","action","restart")<<"</form></section>";}
  else {h<<"<section class='room-head'><p class='eyebrow'>RAVENWOOD ESTATE · SCORE "<<g.score<<"</p><h1>"<<roomName(g.room)<<"</h1></section><section class='scene'><div class='scene-caption'>";
    switch(g.room){case 1:h<<"Portraits watch as candlelight crawls across the grand staircase.";break;case 2:h<<"Torn curtains stir beside the antique mirror. A shadow waits behind you.";break;case 3:h<<"Copper pots sway overhead. The ghost chef guards the cold hearth.";break;case 4:h<<"Tall shelves lean beneath centuries of dust and whispering books.";break;case 5:h<<"Ancient symbols surround a key suspended in a shaft of pale light.";break;}h<<"</div></section><section class='play'>";
    if(g.room>=1&&g.room<=4){
      if(g.pending){const char* lines[]={"","Hey, you are actually very smart!","Keep going! You're doing great!","Come on, you can do it!","You're one step closer to getting out of the haunted mansion!"};h<<"<div class='completion'><p class='eyebrow'>ROOM CLEARED</p><h2>"<<lines[g.room]<<"</h2><p>Your progress is safe. Ready for what waits beyond?</p><form method='post'>"<<button("CONTINUE","action","continue")<<"</form></div>";}
      else if(g.lives>0&&q){const Ghost& ghost=ghostForRoom(g.room);h<<"<p class='eyebrow'>"<<(g.room==3?"THE CHEF'S LAST MEAL":"QUESTION "+std::to_string(g.index+1)+" OF 3")<<"</p><div class='riddle-timer'><span>30 SECONDS · THE GHOST IS CLOSING IN</span><div class='timer-track'><i style='animation-delay:-"<<elapsed<<"s'></i></div></div><div class='ghost-dialogue' aria-live='polite'><span class='ghost-name'>"<<esc(ghost.name())<<"</span><p class='ghost-line'>"<<esc(ghost.greeting())<<"</p><p class='ghost-question'>“"<<esc(q->prompt)<<"”</p></div><form method='post' class='answer'><input name='answer' placeholder='"<<(g.room==3?"Enter dish…":"Enter your answer…")<<"' required autocomplete='off'><button class='button' name='action' value='answer'>SUBMIT</button></form><div class='powers'>"<<hintForm("potion","✦ Magic Potion",g.potionUses)<<hintForm("whisper","◌ Ghost Whisper",g.whisperUses)<<hintForm("answerkey","⚿ Answer Key",g.answerKeyUses)<<"</div>";if(!g.powerMessage.empty())h<<"<p class='power-clue'>"<<esc(g.powerMessage)<<"</p>";h<<"<p class='subtle'>"<<completed(g,g.room)<<" / "<<total(g.room)<<" solved · Incorrect answers cost one life.</p>";}
    } else if(g.room==5){
      if(g.treasureStage==0)h<<"<p class='eyebrow'>A KEY HANGS IN THE COLD AIR</p><p>Collect the old brass key. Something waits beyond the door.</p><form method='post' class='key-form'><button class='key-button' name='action' value='collect-key' aria-label='Collect the key'>⚿<span>COLLECT THE KEY</span></button></form>";
      else if(g.treasureStage==1)h<<"<p class='eyebrow'>THE KEY GROWS IN YOUR HAND</p><p>A giant golden key appears. Click it to unlock the treasure.</p><form method='post' class='key-form'><button class='treasure-action giant-key' name='action' value='turn-key' aria-label='Use the giant key'><span class='treasure-icon'>🗝️</span><span>USE THE KEY</span></button></form>";
      else if(g.treasureStage==2)h<<"<p class='eyebrow'>A TREASURE CHEST APPEARS</p><p>The key has opened a hidden chamber. Open the chest to discover your reward.</p><form method='post' class='key-form'><button class='treasure-action chest-action' name='action' value='open-chest' aria-label='Open the treasure chest'><span class='treasure-icon'>🧰</span><span>OPEN THE CHEST</span></button></form>";
      else h<<"<p class='eyebrow'>YOUR GIFT IS REVEALED</p><div class='gift-reveal'><span class='treasure-icon'>💖</span><h2>One Extra Life</h2><p>A glowing heart rises from the chest. Claim it and escape Ravenwood!</p></div><form method='post' class='key-form'><button class='button' name='action' value='claim-gift'>CLAIM YOUR GIFT &amp; ESCAPE</button></form>";
    }
    h<<"</section><footer><span>ROOM "<<g.room<<" / 5 · "<<g.lives<<" BREATHS LEFT · KEY "<<(g.key?"COLLECTED":"MISSING")<<"</span><form method='post'>"<<button("RESTART","action","restart")<<"</form></footer>";}
  h<<"</main></body></html>";return h.str();
}
void answer(Game&g,const std::string&raw){if(g.lives<=0||g.pending||g.room<1||g.room>4||g.index>=static_cast<int>(g.chosen[g.room-1].size()))return;auto a=norm(raw);const Question&q=g.chosen[g.room-1][g.index];const Ghost&ghost=ghostForRoom(g.room);bool ok=false;for(const auto&x:q.answers)if(a==norm(x))ok=true;if(ok){++g.index;g.score+=100;g.powerMessage.clear();if(g.room==1)g.hallDone=g.index;else if(g.room==2)g.bedDone=g.index;else if(g.room==3)g.kitchenDone=g.index;else if(g.room==4)g.libraryDone=g.index;g.pending=stageComplete(g);g.questionStarted=g.pending?0:std::time(nullptr);g.notice=std::string(ghost.correctReply())+(g.pending?" The room's final secret falls into place.":"");}else{g.lives=std::max(0,g.lives-1);if(g.lives==0)g.questionStarted=0;g.powerMessage.clear();g.notice=std::string(ghost.wrongReply())+(g.lives?" Try again.":" Your last life has faded.");}}
void act(Game&g,const std::map<std::string,std::string>&f){if(f.count("action")&&f.at("action")=="restart"){g=Game{};chooseQuestions(g);g.notice="A new night begins. The house has reshuffled its secrets.";return;}
  if(f.count("action")&&f.at("action")=="start"&&g.room==0){g=Game{};chooseQuestions(g);g.room=-1;g.notice="";return;}
  if(f.count("action")&&f.at("action")=="begin"&&g.room==-1){g.room=1;g.questionStarted=std::time(nullptr);g.notice="The front door slams shut. The ghost is behind you.";return;}
  if(expireIfTimedOut(g))return;
  if(f.count("action")&&f.at("action")=="continue"&&g.pending&&stageComplete(g)){g.pending=false;g.powerMessage.clear();if(g.room==1)g.room=2;else if(g.room==2)g.room=3;else if(g.room==3)g.room=4;else if(g.room==4)g.room=5;g.index=0;g.potionUses=2;g.whisperUses=2;g.whisperRevealed=0;g.questionStarted=(g.room>=1&&g.room<=4)?std::time(nullptr):0;g.notice="You enter "+roomName(g.room)+". Your Magic Potion and Ghost Whisper are refreshed.";return;}
  if(f.count("action")&&f.at("action")=="answer"){answer(g,f.at("answer"));return;}
  if(f.count("answer")){answer(g,f.at("answer"));return;}
  if(f.count("power")&&g.room>=1&&g.room<=4&&!g.pending&&g.lives>0&&g.index<static_cast<int>(g.chosen[g.room-1].size())){const Question&q=g.chosen[g.room-1][g.index];if(f.at("power")=="potion"){if(g.potionUses>0){--g.potionUses;g.powerMessage="Magic Potion: "+q.hint;g.notice="The potion reveals a clue.";}else g.notice="Your Magic Potions are used up for this room.";}else if(f.at("power")=="whisper"){if(g.whisperUses>0){--g.whisperUses;++g.whisperRevealed;const std::string answer=norm(q.answer());const size_t count=std::min(static_cast<size_t>(g.whisperRevealed),answer.size());g.powerMessage="Ghost Whisper: the answer begins with ‘"+answer.substr(0,count)+"’.";g.notice="A ghost whispers another letter.";}else g.notice="Your Ghost Whispers are used up for this room.";}else if(f.at("power")=="answerkey"){if(g.answerKeyUses>0){--g.answerKeyUses;g.powerMessage="Answer Key: the answer is ‘"+q.answer()+"’.";g.notice="An Answer Key reveals the answer. Enter it if you want to solve the question.";}else g.notice="You have used all three Answer Keys for this game.";}return;}
  if(f.count("action")&&f.at("action")=="collect-key"&&g.room==5&&g.treasureStage==0){g.key=true;g.treasureStage=1;g.notice="The key grows huge in your hand.";return;}
  if(f.count("action")&&f.at("action")=="turn-key"&&g.room==5&&g.treasureStage==1){g.treasureStage=2;g.notice="The giant key turns. A treasure chest appears.";return;}
  if(f.count("action")&&f.at("action")=="open-chest"&&g.room==5&&g.treasureStage==2){g.treasureStage=3;g.notice="The chest opens and reveals a glowing extra-life heart.";return;}
  if(f.count("action")&&f.at("action")=="claim-gift"&&g.room==5&&g.treasureStage==3&&!g.escaped){g.score+=500;++g.lives;g.escaped=true;g.room=6;g.notice="You claimed the extra life and escaped Ravenwood!";return;}
  g.notice="That action is unavailable. Complete the current challenge first.";}
}

int main(int argc,char**argv){int port=argc>1?std::atoi(argv[1]):8080;int server=socket(AF_INET,SOCK_STREAM,0);if(server<0){perror("socket");return 1;}int yes=1;setsockopt(server,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes);sockaddr_in addr{};addr.sin_family=AF_INET;addr.sin_addr.s_addr=INADDR_ANY;addr.sin_port=htons(port);if(bind(server,reinterpret_cast<sockaddr*>(&addr),sizeof addr)<0||listen(server,32)<0){perror("server");return 1;}std::cout<<"Ravenwood listening on http://localhost:"<<port<<"\n";
  while(true){int client=accept(server,nullptr,nullptr);if(client<0)continue;std::string req;char buf[4096];int n=recv(client,buf,sizeof buf,0);if(n<=0){close(client);continue;}req.assign(buf,n);auto end=req.find("\r\n\r\n");if(end==std::string::npos){close(client);continue;}std::string head=req.substr(0,end),body=req.substr(end+4),cookie;auto cp=head.find("Cookie:");if(cp!=std::string::npos){auto c=head.find("sid=",cp);if(c!=std::string::npos){c+=4;auto e=head.find_first_of("; \r\n",c);cookie=head.substr(c,e-c);}}if(cookie.empty())cookie=std::to_string(reinterpret_cast<uintptr_t>(&games)^static_cast<uintptr_t>(client)^static_cast<uintptr_t>(std::rand()));size_t lp=head.find("Content-Length:");size_t len=lp==std::string::npos?0:std::strtoul(head.c_str()+lp+15,nullptr,10);while(body.size()<len){n=recv(client,buf,sizeof buf,0);if(n<=0)break;body.append(buf,n);}auto f=fields(body);std::string out;
    if(head.find("GET /style.css ")!=std::string::npos){std::ifstream css("style.css",std::ios::binary);std::stringstream s;s<<css.rdbuf();std::string data=s.str();out="HTTP/1.1 200 OK\r\nContent-Type: text/css; charset=utf-8\r\nContent-Length: "+std::to_string(data.size())+"\r\nConnection: close\r\n\r\n"+data;}
    else if(head.find("GET /assets/ghost-encounter.png ")!=std::string::npos){std::ifstream image("assets/ghost-encounter.png",std::ios::binary);std::stringstream s;s<<image.rdbuf();std::string data=s.str();out="HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nCache-Control: public, max-age=3600\r\nContent-Length: "+std::to_string(data.size())+"\r\nConnection: close\r\n\r\n"+data;}
    else if(head.find("GET /assets/start-background.png ")!=std::string::npos){std::ifstream image("assets/start-background.png",std::ios::binary);std::stringstream s;s<<image.rdbuf();std::string data=s.str();out="HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nCache-Control: public, max-age=3600\r\nContent-Length: "+std::to_string(data.size())+"\r\nConnection: close\r\n\r\n"+data;}
    else if(head.find("GET /assets/hallway-background.png ")!=std::string::npos){std::ifstream image("assets/hallway-background.png",std::ios::binary);std::stringstream s;s<<image.rdbuf();std::string data=s.str();out="HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nCache-Control: public, max-age=3600\r\nContent-Length: "+std::to_string(data.size())+"\r\nConnection: close\r\n\r\n"+data;}
    else if(head.find("GET /assets/bedroom-background.png ")!=std::string::npos){std::ifstream image("assets/bedroom-background.png",std::ios::binary);std::stringstream s;s<<image.rdbuf();std::string data=s.str();out="HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nCache-Control: public, max-age=3600\r\nContent-Length: "+std::to_string(data.size())+"\r\nConnection: close\r\n\r\n"+data;}
    else if(head.find("GET /assets/kitchen-background.png ")!=std::string::npos){std::ifstream image("assets/kitchen-background.png",std::ios::binary);std::stringstream s;s<<image.rdbuf();std::string data=s.str();out="HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nCache-Control: public, max-age=3600\r\nContent-Length: "+std::to_string(data.size())+"\r\nConnection: close\r\n\r\n"+data;}
    else if(head.find("GET /assets/library-background.png ")!=std::string::npos){std::ifstream image("assets/library-background.png",std::ios::binary);std::stringstream s;s<<image.rdbuf();std::string data=s.str();out="HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nCache-Control: public, max-age=3600\r\nContent-Length: "+std::to_string(data.size())+"\r\nConnection: close\r\n\r\n"+data;}
    else if(head.find("GET /assets/victory-background.png ")!=std::string::npos){std::ifstream image("assets/victory-background.png",std::ios::binary);std::stringstream s;s<<image.rdbuf();std::string data=s.str();out="HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nCache-Control: public, max-age=3600\r\nContent-Length: "+std::to_string(data.size())+"\r\nConnection: close\r\n\r\n"+data;}
    else if(head.find("POST ")!=std::string::npos){act(games[cookie],f);out="HTTP/1.1 303 See Other\r\nLocation: /\r\nSet-Cookie: sid="+cookie+"; Path=/; HttpOnly; SameSite=Lax\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";}
    else{Game&g=games[cookie];expireIfTimedOut(g);std::string data=page(g);out="HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nSet-Cookie: sid="+cookie+"; Path=/; HttpOnly; SameSite=Lax\r\nContent-Length: "+std::to_string(data.size())+"\r\nConnection: close\r\n\r\n"+data;}
    send(client,out.data(),out.size(),0);close(client);}
}
