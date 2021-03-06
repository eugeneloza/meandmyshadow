/*
 * Copyright (C) 2011-2013 Me and My Shadow
 *
 * This file is part of Me and My Shadow.
 *
 * Me and My Shadow is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Me and My Shadow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Me and My Shadow.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ADDONS_H
#define ADDONS_H

#include "GameState.h"
#include "GameObjects.h"
#include "GUIObject.h"
#include "GUIListBox.h"
#include <array>
#include <vector>
#include <string>

//The addons menu.
class Addons: public GameState,public GUIEventCallback{
private:
	//The minimum addon version that is supported.
	static const int MIN_VERSION=2;
	//The maximum addon version that is supported.
	static const int MAX_VERSION=2;
	
	//An addon entry.
	struct Addon{
		//The name of the addon.
		string name;
		//The type of addon. (Level, Levelpack, Theme)
		string type;
		//The link to the addon file.
		string file;
		//The name of the author.
		string author;

		//The description of the addon.
		string description;
		//The license of the addon.
		string license;
		//The website of the addon.
		string website;

		//Icon for the addon.
        SDL_Surface* icon;
		//Screenshot for the addon.
        SharedTexture screenshot;
		
		//The latest version of the addon.
		int version;
		//The version that the user has installed, if installed.
		int installedVersion;
		
		//Boolean if the addon is installed.
		bool installed;
		//Boolean if the addon is upToDate. (installedVersion==version)
		bool upToDate;

		//Map that contains the content of the addon.
		//NOTE: This is only filled if the addon is installed.
		std::vector<std::pair<std::string,std::string> > content;
		//Array that holds the name of the addons it's dependent on.
		//NOTE: This is only filled if the addon is installed.
		std::vector<std::pair<std::string,std::string> > dependencies;
	};

	//The title.
    TexturePtr title;

	//Placeholder icons for addons in case they don't provide custom icons.
    std::map<std::string, SDL_Surface*> addonIcon;

	//Placeholder screenshot for addons in case they don't provide one.
    SharedTexture screenshot;

	//Map containing a vector of Addons for each addon category.
	std::vector<Addon> addons;
	
	//String that should contain the error when something fails.
	string error;
	
	//The type of addon that is currently selected.
	string type;
	//Pointer to the addon that is selected.
	Addon* selected;

	//The list used for the selecting of the category.
	GUISingleLineListBox* categoryList;
	//Pointer to the description.
	GUIObject* categoryDescription;
	//The list used for listing the addons.
	GUIListBox* list;
public:
	//Constructor.
    Addons(SDL_Renderer& renderer, ImageManager& imageManager);
	//Destructor.
	~Addons();
	
	//Method that will create the GUI.
    void createGUI(SDL_Renderer &renderer, ImageManager &imageManager);
	
	//Method that loads that downloads the addons list.
	//Returns: True if the file is downloaded successfuly.
    bool getAddonsList(SDL_Renderer& renderer, ImageManager& imageManager);

	void fillAddonList(TreeStorageNode &objAddons,TreeStorageNode &objInstalledAddons,SDL_Renderer& renderer, ImageManager& imageManager);
	//Put all the addons of a given type in a vector.
	//type: The type the addons must be.
	//Returns: Vector containing the addons.
    void addonsToList(const string &type, SDL_Renderer &renderer, ImageManager &);
	
	//Method that will save the installed addons to the installed_addons file.
	//Returns: True if the file is saved successfuly.
	bool saveInstalledAddons();

	//Method for loading a cached image and downloading if it isn't cached.
	//url: The url to the image.
	//md5sum: The md5sum used for caching.
    //Returns: Shared pointer to the loaded image.
    SDL_Surface* loadCachedImage(const char* url,const char* md5sum, ImageManager& imageManager);
	SharedTexture loadCachedTexture(const char* url, const char* md5sum, SDL_Renderer& renderer, ImageManager& imageManager);

	//Method that will open a GUIOverlay with the an overview of the selected addon.
    void showAddon(ImageManager& imageManager,SDL_Renderer& renderer);
	
    //Inherited from GameState.
    void handleEvents(ImageManager&, SDL_Renderer&) override;
    void logic(ImageManager&, SDL_Renderer&) override;
    void render(ImageManager&, SDL_Renderer& renderer) override;
    void resize(ImageManager &imageManager, SDL_Renderer& renderer) override;
	
	//Method used for GUI event handling.
	//name: The name of the callback.
	//obj: Pointer to the GUIObject that caused the event.
	//eventType: The type of event: click, change, etc..
	void GUIEventCallback_OnEvent(ImageManager& imageManager, SDL_Renderer& renderer, std::string name,GUIObject* obj,int eventType);

	//This method will remove the addon based on the content vector.
	//NOTE It doesn't check if the addon is installed or not.
	//addon: The addon to remove.
    void removeAddon(ImageManager& imageManager,SDL_Renderer &renderer, Addon* addon);
	//This method will install the addon by downloading,extracting and reading.
	//NOTE It doesn't check if the addon is installed or not.
	//addon: The addon to install.
    void installAddon(ImageManager& imageManager, SDL_Renderer &renderer, Addon* addon);

};
#endif
