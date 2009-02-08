#include "gitg-preferences-dialog.h"

#include "gitg-preferences.h"
#include "gitg-data-binding.h"

#include <stdlib.h>
#include <glib/gi18n.h>

enum
{
	COLUMN_NAME,
	COLUMN_PROPERTY,
	N_COLUMNS
};

#define GITG_PREFERENCES_DIALOG_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GITG_TYPE_PREFERENCES_DIALOG, GitgPreferencesDialogPrivate))

static GitgPreferencesDialog *preferences_dialog;

struct _GitgPreferencesDialogPrivate
{
	GtkCheckButton *history_search_filter;
	GtkAdjustment *collapse_inactive_lanes;
	GtkTreeView *tree_view_colors;
	
	GtkTable *table_style;
	GtkColorButton *foreground;
	GtkColorButton *background;
	GtkCheckButton *background_line;
	GtkToggleButton *bold;
	GtkToggleButton *italic;
	GtkToggleButton *underline;
	
	gboolean initializing_colors;
	GSList *bindings;
	gint prev_value;
};

G_DEFINE_TYPE(GitgPreferencesDialog, gitg_preferences_dialog, GTK_TYPE_DIALOG)

static gint
round_val(gdouble val)
{
	gint ival = (gint)val;

	return ival + (val - ival > 0.5);
}

static void
free_bindings(GitgPreferencesDialog *dialog)
{
	g_slist_foreach(dialog->priv->bindings, (GFunc)gitg_data_binding_free, NULL);
	g_slist_free(dialog->priv->bindings);
	dialog->priv->bindings = NULL;
}

static void
gitg_preferences_dialog_finalize(GObject *object)
{
	GitgPreferencesDialog *dialog = GITG_PREFERENCES_DIALOG(object);
	
	g_slist_free(dialog->priv->bindings);

	G_OBJECT_CLASS(gitg_preferences_dialog_parent_class)->finalize(object);
}

static void
gitg_preferences_dialog_class_init(GitgPreferencesDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	
	object_class->finalize = gitg_preferences_dialog_finalize;

	g_type_class_add_private(object_class, sizeof(GitgPreferencesDialogPrivate));
}

static void
gitg_preferences_dialog_init(GitgPreferencesDialog *self)
{
	self->priv = GITG_PREFERENCES_DIALOG_GET_PRIVATE(self);
}

static void
on_response(GtkWidget *dialog, gint response, gpointer data)
{
	gtk_widget_destroy(dialog);
}

static gboolean
convert_collapsed(GValue const *source, GValue *dest, gpointer userdata)
{
	GitgPreferencesDialog *dialog = GITG_PREFERENCES_DIALOG(userdata);

	gint val = round_val(g_value_get_double(source));
	
	if (val == dialog->priv->prev_value)
		return FALSE;
	
	dialog->priv->prev_value = val;
	return g_value_transform(source, dest);
}

static void
initialize_view(GitgPreferencesDialog *dialog)
{
	GitgPreferences *preferences = gitg_preferences_get_default();
	
	gitg_data_binding_new_mutual(preferences, "history-search-filter", 
						         dialog->priv->history_search_filter, "active");

	gitg_data_binding_new_mutual_full(preferences, "history-collapse-inactive-lanes",
						              dialog->priv->collapse_inactive_lanes, "value",
						              (GitgDataBindingConversion)g_value_transform,
						              convert_collapsed,
						              dialog);
}

static void
store_add_color(GtkListStore *store, gchar const *name, gchar const *property)
{
	GtkTreeIter iter;
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, COLUMN_NAME, name, COLUMN_PROPERTY, property, -1);
}

static void
add_binding(GitgPreferencesDialog *dialog, GitgDataBinding *binding)
{
	dialog->priv->bindings = g_slist_prepend(dialog->priv->bindings, binding);
}

static gboolean
convert_mask_to_bool(GValue const *mask, GValue *boolean, gpointer m)
{
	g_return_val_if_fail(G_VALUE_HOLDS(mask, G_TYPE_INT), FALSE);
	g_return_val_if_fail(G_VALUE_HOLDS(boolean, G_TYPE_BOOLEAN), FALSE);

	gint mv = g_value_get_int(mask);
	
	g_value_set_boolean(boolean, mv & GPOINTER_TO_INT(m));
	return TRUE;
}

static gboolean
convert_bool_to_mask(GValue const *boolean, GValue *mask, gpointer m)
{
	g_return_val_if_fail(G_VALUE_HOLDS(mask, G_TYPE_INT), FALSE);
	g_return_val_if_fail(G_VALUE_HOLDS(boolean, G_TYPE_BOOLEAN), FALSE);

	gboolean bv = g_value_get_boolean(boolean);
	gint orig = g_value_get_int(mask);
	gint mm = GPOINTER_TO_INT(m);

	if (bv)
		g_value_set_int(mask, orig | mm);
	else
		g_value_set_int(mask, orig & ~mm);

	return TRUE;
}

static void
add_style_binding(GitgPreferencesDialog *dialog, gpointer widget, gchar const *property, GitgPreferencesStyleFlags mask)
{
	gchar *name = g_strconcat(property, "-style", NULL);
	GtkToggleButton *button = GTK_TOGGLE_BUTTON(widget);
	GitgPreferences *preferences = gitg_preferences_get_default();

	GitgDataBinding *binding;
	
	binding = gitg_data_binding_new_mutual_full(preferences, name,
												widget, "active",
												convert_mask_to_bool,
												convert_bool_to_mask,
												GINT_TO_POINTER(mask));
	
	add_binding(dialog, binding);
	g_free(name);
}

static void
on_color_selection_changed(GtkTreeSelection *selection, GitgPreferencesDialog *dialog)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	
	free_bindings(dialog);
	
	if (!gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_widget_set_sensitive(GTK_WIDGET(dialog->priv->table_style), FALSE);
		return;
	}
	
	gtk_widget_set_sensitive(GTK_WIDGET(dialog->priv->table_style), TRUE);
	
	gchar *property;
	gtk_tree_model_get(model, &iter, COLUMN_PROPERTY, &property, -1);
	
	/* create bindings */
	GitgPreferences *preferences = gitg_preferences_get_default();
	GitgDataBinding *binding;

	gchar *cmb = g_strconcat(property, "-foreground", NULL);
	binding = gitg_data_binding_new_mutual_full(preferences, cmb,
												dialog->priv->foreground, "color",
												gitg_data_binding_string_to_color,
												gitg_data_binding_color_to_string,
												NULL);
	add_binding(dialog, binding);
	g_free(cmb);
	
	cmb = g_strconcat(property, "-background", NULL);
	binding = gitg_data_binding_new_mutual_full(preferences, cmb,
												dialog->priv->background, "color",
												gitg_data_binding_string_to_color,
												gitg_data_binding_color_to_string,
												NULL);
	add_binding(dialog, binding);
	g_free(cmb);
	
	add_style_binding(dialog, dialog->priv->bold, property, GITG_PREFERENCES_STYLE_BOLD);
	add_style_binding(dialog, dialog->priv->italic, property, GITG_PREFERENCES_STYLE_ITALIC);
	add_style_binding(dialog, dialog->priv->underline, property, GITG_PREFERENCES_STYLE_UNDERLINE);
	add_style_binding(dialog, dialog->priv->background_line, property, GITG_PREFERENCES_STYLE_LINE_BACKGROUND);
}

static void
initialize_colors(GitgPreferencesDialog *dialog)
{
	GtkListStore *store = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
	
	store_add_color(store, _("Default"), "text");
	store_add_color(store, _("File header"), "header");
	store_add_color(store, _("Chunk header"), "hunk");
	store_add_color(store, _("Added line"), "added-line");
	store_add_color(store, _("Removed line"), "removed-line");
	store_add_color(store, _("Changed line"), "changed-line");
	store_add_color(store, _("Trailing spaces"), "trailing-spaces");
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(dialog->priv->tree_view_colors), GTK_TREE_MODEL(store));
	
	GtkTreeSelection *selection = gtk_tree_view_get_selection(dialog->priv->tree_view_colors);
	g_signal_connect(selection, "changed", G_CALLBACK(on_color_selection_changed), dialog);
}

static void
create_preferences_dialog()
{
	GtkBuilder *b = gtk_builder_new();
	GError *error = NULL;

	if (!gtk_builder_add_from_file(b, GITG_UI_DIR "/gitg-preferences.xml", &error))
	{
		g_critical("Could not open UI file: %s (%s)", GITG_UI_DIR "/gitg-preferences.xml", error->message);
		g_error_free(error);
		exit(1);
	}
	
	preferences_dialog = GITG_PREFERENCES_DIALOG(gtk_builder_get_object(b, "dialog_preferences"));
	g_object_add_weak_pointer(G_OBJECT(preferences_dialog), (gpointer *)&preferences_dialog);
	
	GitgPreferencesDialogPrivate *priv = preferences_dialog->priv;
	priv->history_search_filter = GTK_CHECK_BUTTON(gtk_builder_get_object(b, "check_button_history_search_filter"));
	priv->collapse_inactive_lanes = GTK_ADJUSTMENT(gtk_builder_get_object(b, "adjustment_collapse_inactive_lanes"));
	priv->tree_view_colors = GTK_TREE_VIEW(gtk_builder_get_object(b, "tree_view_colors"));
	
	priv->table_style = GTK_TABLE(gtk_builder_get_object(b, "table_style"));
	priv->foreground = GTK_COLOR_BUTTON(gtk_builder_get_object(b, "color_button_foreground"));
	priv->background = GTK_COLOR_BUTTON(gtk_builder_get_object(b, "color_button_background"));
	priv->background_line = GTK_CHECK_BUTTON(gtk_builder_get_object(b, "check_button_fill_background"));
	priv->bold = GTK_TOGGLE_BUTTON(gtk_builder_get_object(b, "toggle_button_bold"));
	priv->italic = GTK_TOGGLE_BUTTON(gtk_builder_get_object(b, "toggle_button_italic"));
	priv->underline = GTK_TOGGLE_BUTTON(gtk_builder_get_object(b, "toggle_button_underline"));
	
	priv->prev_value = (gint)gtk_adjustment_get_value(priv->collapse_inactive_lanes);
	g_signal_connect(preferences_dialog, "response", G_CALLBACK(on_response), NULL);
	
	initialize_view(preferences_dialog);
	initialize_colors(preferences_dialog);

	gtk_builder_connect_signals(b, preferences_dialog);
	g_object_unref(b);
}

GitgPreferencesDialog *
gitg_preferences_dialog_present(GtkWindow *window)
{
	if (!preferences_dialog)
		create_preferences_dialog();

	gtk_window_set_transient_for(GTK_WINDOW(preferences_dialog), window);
	gtk_window_present(GTK_WINDOW(preferences_dialog));

	return preferences_dialog;
}

void
on_collapse_inactive_lanes_changed(GtkAdjustment *adjustment, GParamSpec *spec, GitgPreferencesDialog *dialog)
{
	gint val = round_val(gtk_adjustment_get_value(adjustment));

	g_signal_handlers_block_by_func(adjustment, G_CALLBACK(on_collapse_inactive_lanes_changed), dialog);
	gtk_adjustment_set_value(adjustment, val);
	g_signal_handlers_unblock_by_func(adjustment, G_CALLBACK(on_collapse_inactive_lanes_changed), dialog);
}
