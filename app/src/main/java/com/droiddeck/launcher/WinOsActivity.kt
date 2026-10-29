package com.droiddeck.launcher

import android.app.Activity
import android.content.Intent
import android.graphics.Color
import android.graphics.Typeface
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.Gravity
import android.view.View
import android.widget.Button
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TextView
import android.widget.Toast
import com.droiddeck.launcher.core.PhantomProcessLimit
import com.droiddeck.launcher.files.InAppFilePicker
import com.droiddeck.launcher.frontend.AddedGames
import com.droiddeck.launcher.runtime.LinuxRuntimeInstaller
import com.droiddeck.launcher.session.NoSteamLaunch
import com.droiddeck.launcher.session.SessionService
import com.droiddeck.launcher.session.SessionState
import org.json.JSONArray
import java.io.File

/** A short list of Windows programs that start through droiddeck-proton-run, without the Steam client. */
class WinOsActivity : Activity() {
    private lateinit var list: LinearLayout
    private lateinit var status: TextView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val pad = (20 * resources.displayMetrics.density).toInt()
        val root = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(pad, pad * 2, pad, pad)
            setBackgroundColor(Color.parseColor("#0E1116"))
        }
        root.addView(text("Droid-WinOs", 26f, bold = true))
        root.addView(text("Windows programlarını Steam olmadan çalıştır.", 14f, "#9AA4B2"))
        status = text("", 13f, "#E0A050").apply { setPadding(0, pad / 2, 0, pad / 2) }
        root.addView(status)
        root.addView(Button(this).apply {
            text = "Program ekle (.exe)"
            setOnClickListener { pickProgram() }
        })
        list = LinearLayout(this).apply { orientation = LinearLayout.VERTICAL }
        root.addView(ScrollView(this).apply { addView(list) },
            LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, 0, 1f))
        root.addView(Button(this).apply {
            text = "DroidDeck ana ekranı (kurulum ve ayarlar)"
            setOnClickListener { startActivity(Intent(this@WinOsActivity, FrontEndActivity::class.java)) }
        })
        setContentView(root)
    }

    override fun onResume() {
        super.onResume()
        refreshStatus()
        refreshList()
    }

    private fun text(value: String, size: Float, color: String = "#FFFFFF", bold: Boolean = false) =
        TextView(this).apply {
            text = value
            textSize = size
            setTextColor(Color.parseColor(color))
            if (bold) setTypeface(typeface, Typeface.BOLD)
        }

    private fun refreshStatus() {
        val runtime = runCatching { LinuxRuntimeInstaller.installedVersion(this) }.getOrNull()
        val phantom = runCatching { PhantomProcessLimit.read(this) }.getOrNull()
        status.text = when {
            runtime == null -> "Linux çalışma ortamı kurulu değil. Aşağıdaki DroidDeck ana ekranından kur."
            phantom != null && PhantomProcessLimit.blocksSteam(phantom) ->
                "Uyarı: Android arka plan süreç sınırı programları kapatabilir (" +
                    PhantomProcessLimit.title(this, phantom) + ")."
            else -> ""
        }
        status.visibility = if (status.text.isEmpty()) View.GONE else View.VISIBLE
    }

    private fun saved(): MutableList<String> {
        val raw = getSharedPreferences(PREFS, MODE_PRIVATE).getString(KEY, "[]").orEmpty()
        val out = ArrayList<String>()
        runCatching { JSONArray(raw) }.getOrNull()?.let { a -> for (i in 0 until a.length()) out.add(a.optString(i)) }
        return out
    }

    private fun save(paths: List<String>) {
        getSharedPreferences(PREFS, MODE_PRIVATE).edit().putString(KEY, JSONArray(paths).toString()).apply()
    }

    private fun refreshList() {
        list.removeAllViews()
        val paths = saved()
        if (paths.isEmpty()) {
            list.addView(text("Henüz program yok. \"Program ekle\" ile bir .exe seç.", 14f, "#9AA4B2"))
            return
        }
        for (path in paths) {
            list.addView(Button(this).apply {
                text = File(path).name + (if (File(path).isFile) "" else "  (dosya bulunamadı)")
                isAllCaps = false
                gravity = Gravity.START or Gravity.CENTER_VERTICAL
                setOnClickListener { run(path) }
                setOnLongClickListener {
                    save(saved().also { it.remove(path) })
                    refreshList()
                    true
                }
            })
        }
        list.addView(text("Silmek için bir programa uzun bas.", 12f, "#6B7686"))
    }

    private fun pickProgram() {
        val start = File(Environment.getExternalStorageDirectory(), "Download").path
        startActivityForResult(InAppFilePicker.buildIntent(this, listOf("exe"), "Çalıştırılacak programı seç", start), REQ_PICK)
    }

    @Deprecated("Deprecated in Java")
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode != REQ_PICK || resultCode != RESULT_OK) return
        val path = InAppFilePicker.pickedPath(data) ?: return
        val paths = saved()
        if (path !in paths) save(paths + path)
        refreshList()
    }

    private fun run(path: String) {
        val exe = File(path)
        if (!exe.isFile) {
            toast("Dosya bulunamadı: ${exe.name}")
            return
        }
        if (SessionState.running) {
            toast("Zaten bir oturum açık. DroidDeck ana ekranından kapat ya da ona dön.")
            return
        }
        if (runCatching { LinuxRuntimeInstaller.installedVersion(this) }.getOrNull() == null) {
            toast("Önce Linux çalışma ortamını kur (DroidDeck ana ekranı).")
            return
        }
        val app = applicationContext
        Thread({
            val result = try {
                AddedGames.addExe(app, exe)
            } catch (e: Exception) {
                Log.w(TAG, "add exe: ${e.message}")
                null
            }
            val game = when (result) {
                is AddedGames.AddResult.Added -> result.game
                is AddedGames.AddResult.Existing -> result.game
                is AddedGames.AddResult.Switched -> result.game
                else -> null
            }
            runOnUiThread {
                if (isDestroyed) return@runOnUiThread
                if (game == null) {
                    toast(
                        if (result is AddedGames.AddResult.Unreachable) "Bu klasöre Linux ortamından erişilemiyor. Programı Download gibi bir klasöre koy."
                        else "Program eklenemedi."
                    )
                    return@runOnUiThread
                }
                val command = NoSteamLaunch.addedCommand(game)
                NoSteamLaunch.remember(command)
                startActivity(
                    Intent(this, SessionActivity::class.java)
                        .putExtra(SessionService.EXTRA_MODE, SessionService.MODE_RUN)
                        .putExtra(SessionService.EXTRA_PROGRAM, command.program)
                        .putExtra(SessionService.EXTRA_PROGRAM_ARGS, command.args.toTypedArray())
                        .putExtra(SessionService.EXTRA_GAME_ID, NoSteamLaunch.settingsId(game).toString())
                )
            }
        }, "winos-run").start()
    }

    private fun toast(message: String) = Toast.makeText(this, message, Toast.LENGTH_LONG).show()

    private companion object {
        const val TAG = "WinOs"
        const val PREFS = "winos"
        const val KEY = "programs"
        const val REQ_PICK = 4201
    }
}
