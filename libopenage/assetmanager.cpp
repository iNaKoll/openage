// Copyright 2014-2016 the openage authors. See copying.md for legal info.

#include "assetmanager.h"

#if WITH_INOTIFY
#include <sys/inotify.h>
#include <unistd.h>
#include <limits.h> /* for NAME_MAX */
#endif

#include "util/compiler.h"
#include "util/file.h"
#include "error/error.h"
#include "log/log.h"

#include "texture.h"

namespace openage {

AssetManager::AssetManager(qtsdl::GuiItemLink *gui_link)
	:
	root{std::string()},
	missing_tex{nullptr},
	gui_link{gui_link} {

#if WITH_INOTIFY
	// initialize the inotify instance
	this->inotify_fd = inotify_init1(IN_NONBLOCK);
	if (this->inotify_fd < 0) {
		throw Error{MSG(err) << "Failed to initialize inotify!"};
	}
#endif
}

util::Dir *AssetManager::get_data_dir() {
	return &this->root;
}

std::string AssetManager::get_data_dir_string() const {
	return this->root.basedir;
}

void AssetManager::set_data_dir_string(const std::string& data_dir) {
	if (this->root.basedir != data_dir) {
		this->root.basedir = data_dir;
		this->clear();
	}
}

bool AssetManager::can_load(const std::string &name) const {
	return util::file_size(this->root.join(name)) > 0;
}

std::shared_ptr<Texture> AssetManager::load_texture(const std::string &name, bool use_metafile) {
	std::string filename = this->root.join(name);

	// the texture to be associated with the given filename
	std::shared_ptr<Texture> tex;

	// try to open the texture filename.
	if (not this->can_load(name)) {
		log::log(MSG(warn) <<  "   file " << filename << " is not there...");

		// TODO: add/fetch inotify watch on the containing folder
		// to display the tex as soon at it exists.

		// return the big X texture instead
		tex = this->get_missing_tex();
	} else {
		// create the texture!
		tex = std::make_shared<Texture>(filename, use_metafile);

#if WITH_INOTIFY
		// create inotify update trigger for the requested file
		int wd = inotify_add_watch(this->inotify_fd, filename.c_str(), IN_CLOSE_WRITE);
		if (wd < 0) {
			throw Error{MSG(warn) << "Failed to add inotify watch for " << filename};
		}
		this->watch_fds[wd] = tex;
#endif
	}

	// insert the texture into the map and return the texture.
	this->textures[filename] = tex;

	// pass back the shared_ptr<Texture>
	return tex;
}

Texture *AssetManager::get_texture(const std::string &name, bool use_metafile) {
	// check whether the requested texture was loaded already
	auto tex_it = this->textures.find(this->root.join(name));

	// the texture was not loaded yet:
	if (tex_it == this->textures.end()) {
		return this->load_texture(name, use_metafile).get();
	}

	return tex_it->second.get();
}

std::vector<gamedata::palette_color> AssetManager::get_palette(const std::string &name) {
	auto pal_it = this->palettes.find(this->root.join(name));

	if (pal_it == std::end(this->palettes)) {
		auto &pal = this->palettes[name];
		util::read_csv_file(this->root.join(name), pal);
		return pal;
	}

	return pal_it->second;
}

void AssetManager::check_updates() {
#if WITH_INOTIFY
	// buffer for at least 4 inotify events
	char buf[4 * (sizeof(struct inotify_event) + NAME_MAX + 1)];
	ssize_t len;

	while (true) {
		// fetch all events, the kernel won't write "half" structs.
		len = read(this->inotify_fd, buf, sizeof(buf));

		if (len == -1 and errno == EAGAIN) {
			// no events, nothing to do.
			break;
		}
		else if (len == -1) {
			throw Error{MSG(err) << "Failed to read inotify events!"};
		}

		// process fetched events,
		// the kernel guarantees complete events in the buffer.
		char *ptr = buf;
		while (ptr < buf + len) {
			struct inotify_event *event = (struct inotify_event *)ptr;

			if (event->mask & IN_CLOSE_WRITE) {
				// TODO: this should invoke callback functions
				this->watch_fds[event->wd]->reload();
			}

			// move the buffer ptr to the next event.
			ptr += sizeof(struct inotify_event) + event->len;
		}
	}
#endif
}

std::shared_ptr<Texture> AssetManager::get_missing_tex() {

	// if not loaded, fetch the "missing" texture (big red X).
	if (unlikely(this->missing_tex.get() == nullptr)) {
		this->missing_tex = std::make_shared<Texture>(root.join("missing.png"), false);
	}

	return this->missing_tex;
}

void AssetManager::clear() {
#if WITH_INOTIFY
	for (auto& watch_fd : this->watch_fds) {
		int result = inotify_rm_watch(this->inotify_fd, watch_fd.first);
		if (result < 0) {
			throw Error{MSG(warn) << "Failed to remove inotify watch"};
		}
	}
	this->watch_fds.clear();
#endif

	this->palettes.clear();
	this->textures.clear();
}

}
