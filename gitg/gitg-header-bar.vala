/*
 * This file is part of gitg
 *
 * Copyright (C) 2013 - Ignacio Casal Quinteiro
 *
 * gitg is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * gitg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gitg. If not, see <http://www.gnu.org/licenses/>.
 */

namespace Gitg
{

[GtkTemplate (ui = "/org/gnome/gitg/ui/gitg-header-bar.ui")]
public class HeaderBar : Gtk.HeaderBar
{
	private Mode d_mode;

	[GtkChild]
	private Gtk.MenuButton d_gear_menu;
	private MenuModel d_dash_model;
	private MenuModel d_activities_model;

	[GtkChild]
	private Gtk.Button d_dash_button;

	[GtkChild]
	private Gtk.ToggleButton d_search_button;

	[GtkChild]
	private Gtk.Button d_commit_button;

	[GtkChild]
	private Gtk.Button d_done_button;

	[GtkChild]
	private Gtk.Separator d_close_button_separator;
	[GtkChild]
	private Gtk.Button d_close_button;

	public signal void request_dash();
	public signal void request_commit();
	public signal void exit_mode();

	public enum Mode
	{
		DASH,
		ACTIVITIES,
		COMMIT
	}

	[CCode (notify = false)]
	public Mode mode
	{
		get { return d_mode; }
		set
		{
			d_mode = value;

			if (d_mode == Mode.COMMIT)
			{
				get_style_context().add_class("selection-mode");

				d_gear_menu.hide();
				d_dash_button.hide();
				d_search_button.hide();
				d_commit_button.hide();
				d_close_button_separator.hide();
				d_close_button.hide();
				d_done_button.show();
			}
			else
			{
				get_style_context().remove_class("selection-mode");

				d_gear_menu.show();
				d_search_button.show();
				d_close_button_separator.show();
				d_close_button.show();
				d_done_button.hide();
			}

			if (d_mode == Mode.DASH)
			{
				d_gear_menu.menu_model = d_dash_model;
				d_dash_button.hide();
				d_commit_button.hide();
			}
			else if (d_mode == Mode.ACTIVITIES)
			{
				d_gear_menu.menu_model = d_activities_model;
				d_dash_button.show();
				d_commit_button.show();
			}

			notify_property("mode");
		}
	}

	[GtkCallback]
	private void close_button_clicked(Gtk.Button button)
	{
		Gtk.Window window = (Gtk.Window)get_toplevel();
		window.close();
	}

	[GtkCallback]
	private void dash_button_clicked(Gtk.Button button)
	{
		request_dash();
	}

	[GtkCallback]
	private void done_button_clicked(Gtk.Button button)
	{
		exit_mode();
	}

	[GtkCallback]
	private void commit_button_clicked(Gtk.Button button)
	{
		request_commit();
	}

	construct
	{
		string menuname;

		if (Gtk.Settings.get_default().gtk_shell_shows_app_menu)
		{
			menuname = "win-menu";
		}
		else
		{
			menuname = "app-win-menu";
		}

		d_dash_model = Resource.load_object<MenuModel>("ui/gitg-menus.ui", menuname + "-dash");
		d_activities_model = Resource.load_object<MenuModel>("ui/gitg-menus.ui", menuname + "-views");
	}

	public Gtk.ToggleButton get_search_button()
	{
		return d_search_button;
	}
}

}

// ex:ts=4 noet
