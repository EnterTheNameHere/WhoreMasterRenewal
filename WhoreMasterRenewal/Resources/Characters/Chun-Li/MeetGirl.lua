
local event_stage

local function dump_stats(obj)
	wm.log(string.format("Charisma=%d", obj.charisma))
	wm.log(string.format("Happiness=%d", obj.happiness))
	wm.log(string.format("Libido=%d", obj.libido))
	wm.log(string.format("Constitution=%d", obj.constitution))
	wm.log(string.format("Intelligence=%d", obj.intelligence))
	wm.log(string.format("Confidence=%d", obj.confidence))
	wm.log(string.format("Mana=%d", obj.mana))
	wm.log(string.format("Agility=%d", obj.agility))
	wm.log(string.format("Fame=%d", obj.fame))
	wm.log(string.format("Level=%d", obj.level))
	wm.log(string.format("Ask Price=%d", obj.askprice))
	wm.log(string.format("House=%d", obj.house))
	wm.log(string.format("Exp=%d", obj.exp))
	wm.log(string.format("Age=%d", obj.age))
	wm.log(string.format("Obedience=%d", obj.obedience))
	wm.log(string.format("Spirit=%d", obj.spirit))
	wm.log(string.format("Beauty=%d", obj.beauty))
	wm.log(string.format("Tiredness=%d", obj.tiredness))
	wm.log(string.format("Health=%d", obj.health))
	wm.log(string.format("PCFear=%d", obj.pc_fear))
	wm.log(string.format("PCLove=%d", obj.pc_love))
	wm.log(string.format("PCHate=%d", obj.pc_hate))
end

local function dump_skills(obj)
	wm.log(string.format("anal=%d", obj.anal))
	wm.log(string.format("magic=%d", obj.magic))
	wm.log(string.format("bdsm=%d", obj.bdsm))
	wm.log(string.format("normal=%d", obj.normal))
	wm.log(string.format("beastiality=%d", obj.beastiality))
	wm.log(string.format("group=%d", obj.group))
	wm.log(string.format("lesbian=%d", obj.lesbian))
	wm.log(string.format("service=%d", obj.service))
	wm.log(string.format("strip=%d", obj.strip))
	wm.log(string.format("combat=%d", obj.combat))
end

local function dump_player()
	assert(wm.player ~= nil)
	local p = wm.player
	wm.log("Player dump BEGIN")
	wm.log(string.format("won=%d", p.win_flag))
	wm.log(string.format("sus=%d", p.suspicion))
	wm.log(string.format("disp=%d", p.disposition))
	wm.log(string.format("fear=%d", p.customer_fear))
	dump_stats(p)
	dump_skills(p)
	wm.log("Player dump END")
end



local function stage_start()
	wm.log(string.format("encounter script for %q", wm.girl.name))
--
--	a bit of clutter for the log file so I can test the 
--	wm.girl values
--
	wm.log(string.format("Name %q", wm.girl.name))
	wm.log(string.format("Real Name %q", wm.girl.real_name))
	wm.log(string.format("Description %q", wm.girl.desc))

	dump_stats(wm.girl)
	dump_skills(wm.girl)

	local k,v
	wm.log("Traits BEGIN")
	for k,v in pairs(wm.girl.traits) do
		wm.log(string.format("%s : %s", k, v))
	end
	wm.log("Traits END")

	dump_player()

	wm.message(
		"The market beggars are getting worse. Each time you come " ..
		"here, it seems there are more of them. After a " ..
		"while, the mind starts filtering them out. It's like the "..
		"ticking sounds from a familiar clock. After a while they "..
		"just drop from conscious awareness. It's sad really."
	, 0)
	wm.message("An insistent tugging startles you from your musings.", 0)
	wm.message("'Please. I am hungry. I need to eat!'", 0)
	wm.message("It seems the beggars are also growing bolder." , 0)
	wm.menu {
		--
		-- be nice to offer an option to give the beggar a coin here
		-- maybe reward that with a small boost to PCLove
		--
		-- means I need to get the girl stats sorted out
		-- as well as the update mechanism
		--
		captions = {
			"Strike the begger for his impertinence",
			"Take a closer look"
		},
		callback = function(choice)
			if choice == 1 then
				event_stage = "violence"
			elseif choice == 2 then
				event_stage = "brains"
			else
				wm.log("unexpected choice in menu callback")
			end
		end
	}
end

local function stage_violence()
	wm.message(
		"'You need to learn to respect your betters, beggar!' ", 0
	)
	wm.message(
		"You lash out backhanded, but the blow never lands. "..
		"Instead you find yourself flying backwards through " ..
		"the air to land, undignified, in the mud",
		0
	)
	wm.message(
		"'Well?' you shout at the pair of goons you brought as " ..
		"bodyguards. 'Don\'t just stand there with your mouths " ..
		"hanging open!'",
		0
	)
	wm.message(
		"The goons set off in hot pursuit, but you doubt they'll " ..
		"catch anyone in the crowded market. Belatedly, it occurs "..
		"to that the beggar girl had a great pair of legs. " ..
		"Perhaps you missed an opportunity here.",
		0
	)
	return false
end

local function stage_brains()
	wm.message(
		"Mastering your irritation, you stop and take a " ..
		"good look at the beggar. It's a woman and younger than " ..
		"you first thought. This could be interesting... ",
		0
	)
	wm.message(
		"'I'm sorry love',  you tell her, 'but it's all I can do "..
		"to feed the girls that work for me. I can't afford to go "..
		"giving handouts to beggars. Sorry.'",
		0
	)
	wm.message(
		"'I don't want a handout! I can earn my keep. I can fight. "..
		"I can fight well!'",
		0
	)
	wm.message(
		"Hmmm... good bone structure too. Definite possibilities " ..
		"here.",
		0
	)
	wm.message(
		"'I dare say you can doll, but I'm not looking for guards. "..
		"Muscle is cheap in Crossgate, and I prefer to hire people "..
		"I know.'",
		0
	)
	wm.message(
		"'Yes, all the people I talk to say the same thing. But " ..
		"still I must eat. These 'girls' you mention - what is it "..
		"they do for you? If they do it, I can do it too",
		0
	)
	wm.message("Hmmm .... Let's *talk* about that, shall we?", 0)
	wm.log(string.format("adding %q to brothel", wm.girl.name))
	wm.add_girl_to_brothel(wm.girl)
	return false
end

function init()
	wm.log("encounter init")

	local k,v

	wm.log("### wm dump")
	for k,v in pairs(wm) do
		wm.log(k,v)
	end
	wm.log("### end wm dump")

	event_stage = "start"
	-- return true - or the run func won't work
	return true
end

function run()
	if event_stage == "start" then
		stage_start()
		return true
	elseif event_stage == "violence" then
		stage_violence()
		return false
	elseif event_stage == "brains" then
		stage_brains()
		return false
	else
		wm.log("unexpected event stage: " .. tostring(event_stage))
		return false
	end
end

