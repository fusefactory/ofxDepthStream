#ofxCMS

Key-value based Model and Collection classes for OpenFrameworks, inspired by Backbone.js

## Features

* Parse JSON
* Find and update existing models or create new ones
* String-based key-value attributes
* Event hooks for collection and attribute changes
* Auto-syncing collections
* Auto-filtering collections

## Quick Start


	CMS::Collection<CMS::Model> records;
	CMS::Model *record;

	record = new CMS::Model();
	record->set("id", "1");
	record->set("name", "Bill");
	record->set("sex", "male");
	records.add(record);

	record = new CMS::Model();
	record->set("id", "2");
	record->set("name", "Jane");
	record->set("sex", "female");
	records.add(record);

	records.filterBy("sex", "female");

	for(int i=0; i<records.count(); i++){
		ofLog() << records.at(i)->get("name");
	}

